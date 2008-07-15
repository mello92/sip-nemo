#include "mysip-app.h"
#include "random.h"

static class mysipAppClass : public TclClass {
public:
	mysipAppClass() : TclClass("Application/mysipApp") {}
	TclObject* create(int, const char* const*) {
		return (new mysipApp);
	}
} class_app_mysip;

void SendTimer::expire(Event*)
{
	printf("SendTimer expire..\n");
	//t_->send_mm_pkt();
	t_->send_mysip_data();
}

void AckTimer::expire(Event*)
{
	printf("AckTimer expire..\n");
	//t_->send_ack_pkt();
	t_->send_ack_for_200ok_pkt();
}

void OKTimer::expire(Event*)
{
	printf("OKTimer expire..\n");
	t_->send_200ok_pkt();
}

void InviteTimer::expire(Event*)
{
	printf("InviteTimer expire..\n");
	t_->send_invite_pkt();
}

mysipApp::mysipApp() :running_(0), snd_timer_(this), ack_timer_(this), ok_timer_(this), 
	invite_timer_(this), CSeq_(1), invite_stop(0), m200ok_stop(0), ack_stop(0), 
	error_recover_num(0), last_reinvite_time(0)
{
	bind_bw("rate0_",&rate[0]);
	bind_bw("rate1_",&rate[1]);
	bind_bw("rate2_",&rate[2]);
	bind_bw("rate3_",&rate[3]);
	bind_bw("rate4_",&rate[4]);
	bind("pktsize_",&pktsize_);
	bind_bool("random_",&random_);
	
	LIST_INIT(&url_head);
}

int mysipApp::command(int argc, const char* const* argv)
{
	Tcl& tcl = Tcl::instance();
	if (argc == 2) {
		if (strcmp(argv[1], "registration") == 0) {
			send_register_pkt();
			tcl.resultf("%d@%s do registration\n",myID,PRINTADDR(myURL));
			return(TCL_OK);
		} 
		else if (strcmp(argv[1], "dump_handoff_info") == 0) {
			dump_handoff();
			return(TCL_OK);
		}
	} 
	else if (argc==3) {
		if (strcmp(argv[1], "attach-agent") == 0) {
			agent_ = (Agent*) TclObject::lookup(argv[2]);
			if (agent_==0) {
				tcl.resultf("no such agent %s", argv[2]);
				return(TCL_ERROR);
			}
			if (agent_->supportMM()) {
				agent_->enableMM();
			}
			else {
				tcl.resultf("agent \"%s\" dose not support MM Applicaiton", argv[2]);
				return(TCL_ERROR);
			}
			agent_->attachApp(this);
			return(TCL_OK);
		}
	} 
	else if (argc==4) {
		if (strcmp(argv[1], "myID_URL") == 0) {
			myID = atoi(argv[2]);
			myURL = STR2ADDR(argv[3]);
			//myID_URL();
			printf("myURL %d@%d\n",myID,myURL);
			tcl.resultf("myURL is %d@%s\n",myID, PRINTADDR(myURL));
			return(TCL_OK);
		}
		else if (strcmp(argv[1], "send_invite") == 0) {
			toID = atoi(argv[2]);
			toURL = STR2ADDR(argv[3]);
			start();
			tcl.resultf("%d@%s to URL is %d%%s\n", myID, PRINTADDR(myURL), toID, PRINTADDR(toURL));
			return(TCL_OK);
		}
		else if (strcmp(argv[1], "log_file") == 0) {
			strcpy(filename,argv[2]);
			strcpy(filename_sec,argv[3]);
			return(TCL_OK);
		}
	}
	else if (argc==5) {
		if (strcmp(argv[1], "add_URL_record") == 0) {
			int xurl_id = atoi(argv[2]);
			int xurl = STR2ADDR(argv[3]);
			int xip = STR2ADDR(argv[4]);
			add_urllist(xurl_id, xurl, xip, 3);
			return(TCL_OK);
		}
	}
	
	return (Application::command(argc, argv));
}

void mysipApp::init()
{
	scale_ = 0;
	seq_ = 0;
	interval_ = (double)(pktsize_<<3)/(double)rate[scale_];
	printf("interval %f rate[scale_] %f\n", interval_ , rate[scale_]);
}

void mysipApp::start()
{
	init();
	cout << "myURL is " << myID << "@" << PRINTADDR(myURL) << endl;
	running_ = 1;
	contact_dst_addr = toURL;
	send_invite_pkt();
	//send_mm_pkt();
}

void mysipApp::stop()
{
	running_ = 0;
}

void mysipApp::send_register_pkt()
{
	cout << NOW << " sipA::register c_dst_addr " << contact_dst_addr << endl;
	
	hdr_mysip register_buf;
	register_buf.ack = 1;
	register_buf.time = NOW;
	register_buf.seq = -2;
	register_buf.nbytes = 200;
	register_buf.scale = p_accnt.last_scale;
	register_buf.method = 3;
	register_buf.requestURL = myURL;
	register_buf.requestURL_id = 88;
	register_buf.From_id = myID;
	register_buf.From = myURL;
	register_buf.To_id = myID;
	register_buf.To = myURL;
	
	register_buf.CSeq = CSeq_++;
	register_buf.contact_id = myID;
	register_buf.contact = agent_->addr();
	register_buf.cport = 3;
	
	struct hdr_mysip* temp = &register_buf;
	show_sipheader(temp);
	
	agent_->sendmsg(register_buf.nbytes, (char*)&register_buf);
}

void mysipApp::show_sipheader(hdr_mysip* msg)
{
	cout << "sipH method: " << msg->method << "\t" 
				<< "reqURL: " << msg->requestURL_id << "@" << PRINTADDR(msg->requestURL) << endl 
				<< "From: " << msg->From_id << "@" << PRINTADDR(msg->From) << endl
				<< "To: " << msg->To_id << "@" << PRINTADDR(msg->To) << endl
				<< "contact: " << msg->contact_id << "@" << PRINTADDR(msg->contact) << endl
				<< "CSeq: " << msg->CSeq << endl
				<< "cip: " << PRINTADDR(msg->cip) << endl
				<< "cport: " << msg->cport << endl
				<< endl;
}

void mysipApp::send_mysip_data()
{
	cout << NOW << " mysipApp::send_mysip_data c_dst_addr " << contact_dst_addr << endl;
	hdr_mysip sip_buf;
	
	if (running_) {
		
		sip_buf.ack = 0;
		sip_buf.seq = seq_++;
		sip_buf.nbytes = pktsize_;
		sip_buf.time = Scheduler::instance().clock();
		sip_buf.scale = scale_;
		sip_buf.method = 1;
		sip_buf.requestURL =  contact_dst_addr;
		sip_buf.From = agent_->addr();
		sip_buf.To = agent_->daddr();
		sip_buf.CSeq = 1;
		sip_buf.contact = agent_->addr();
		
		agent_->sendmsg(pktsize_, (char*)&sip_buf);
		
		snd_timer_.resched(0.01);
	}
}

void mysipApp::send_ack_for_200ok_pkt()
{
	cout << NOW << " sipA::ack c_dst_addr " << contact_dst_addr << endl;
	
	hdr_mysip ack_buf;
	memcpy(&ack_buf,last_siph,sizeof(struct hdr_mysip));
	ack_buf.ack = 1;
	ack_buf.time = NOW;
	ack_buf.seq = -2;
	ack_buf.nbytes = 40;
	ack_buf.scale = p_accnt.last_scale;
	ack_buf.method = 2;
	ack_buf.From = agent_->addr();
	ack_buf.To = agent_->daddr();
	ack_buf.CSeq = CSeq_++;
	ack_buf.contact = agent_->addr();
	ack_buf.requestURL = contact_dst_addr;
	
	agent_->sendmsg(ack_buf.nbytes, (char*)&ack_buf);
	
}

void mysipApp::send_temp_move_pkt()
{
	cout << NOW << " sipA::send temp_move c_dst_addr " << contact_dst_addr << endl;
	
	hdr_mysip tempmove_buf;
	memcpy(&tempmove_buf, last_siph, sizeof(struct hdr_mysip));
	tempmove_buf.ack = 1;
	tempmove_buf.time = NOW;
	tempmove_buf.seq = -2;
	tempmove_buf.nbytes = 300;
	tempmove_buf.scale = p_accnt.last_scale;
	tempmove_buf.method = 4;
	tempmove_buf.requestURL = last_siph ->contact;
	tempmove_buf.requestURL_id = last_siph ->contact_id;
	URLIPtable* urlentry = lookup_table(headu(url_head), last_siph->requestURL_id, last_siph->requestURL);
	if (myID==88 && urlentry)
	{
		tempmove_buf.contact = urlentry->IP;
		show_sipheader(&tempmove_buf);
		agent_->sendmsg(tempmove_buf.nbytes, (char*) &tempmove_buf);
	}
}

void mysipApp::send_200ok_pkt()
{
	cout << NOW << " sipA:200ok c_dst_addr " << contact_dst_addr << endl;
	adjust_scale();
	
	hdr_mysip ok_buf;
	memcpy(&ok_buf, last_siph, sizeof(struct hdr_mysip));
	ok_buf.ack = 1;
	ok_buf.time = NOW;
	ok_buf.seq = -2;
	ok_buf.nbytes = 80;
	ok_buf.scale = p_accnt.last_scale;
	ok_buf.method = 1;
	ok_buf.CSeq = CSeq_++;
	ok_buf.contact_id = myID;
	ok_buf.contact = agent_->addr();
	ok_buf.requestURL = contact_dst_addr;
	ok_buf.cport = 3;
	show_sipheader(&ok_buf);
	agent_->sendmsg(ok_buf.nbytes, (char*)&ok_buf);
	
}

void mysipApp::send_invite_pkt()
{
	cout << NOW << " sipA::invite c_dst_addr " << contact_dst_addr << endl;
	
	hdr_mysip invite_buf;
	invite_buf.ack = 1;
	invite_buf.time = NOW;
	invite_buf.seq = -2;
	invite_buf.nbytes = 120;
	invite_buf.scale = p_accnt.last_scale;
	invite_buf.method = 0;
	invite_buf.From_id = myID;
	invite_buf.From = myURL;
	invite_buf.To_id = toID;
	invite_buf.To = toURL;
	invite_buf.CSeq = CSeq_ ++;
	invite_buf.contact_id = myID;
	invite_buf.contact = agent_->addr();
	invite_buf.requestURL_id = toID;
	invite_buf.requestURL = contact_dst_addr;
	invite_buf.cip = agent_->addr();
	invite_buf.cport = 3;
	show_sipheader(&invite_buf);
	agent_->sendmsg(invite_buf.nbytes, (char*)&invite_buf);
	//cout << "sipA::invite next_time: " << p_accnt.rtt << "invite_stop: " << invite_stop << endl;
	//if(!invite_stop)
	//	invite_timer_.resched(2);
}

void mysipApp::send_mm_pkt()
{
	
	
	hdr_mysip mh_buf;
	
	if (running_) {
		mh_buf.ack = 0;
		mh_buf.seq = seq_++;
		mh_buf.nbytes = pktsize_;
		mh_buf.time = Scheduler::instance().clock();
		mh_buf.scale = scale_;
		
		printf("---%f send_mm_pkt\n",mh_buf.time);
		agent_->sendmsg(pktsize_, (char*) &mh_buf);
		
		double next_time_ = next_snd_time();
		printf("next_time %f\n", next_time_);
		if(next_time_ > 0) 
			snd_timer_.resched(next_time_);
	}
}

double mysipApp::next_snd_time()
{
	interval_ = (double)(pktsize_ << 3)/(double)rate[scale_];
	printf("interval %f rate[scale_] %f\n", interval_ , rate[scale_]);
	double next_time_ = interval_;
	if(random_)
		next_time_ +=interval_*Random::uniform(-0.5,0.5);
	return next_time_;
}

void mysipApp::recv_msg(int bytes, const char *msg)
{
	cout << NOW << " mysipApp::recv_msg " << endl;
	
	if (msg) {
		hdr_mysip* sip_buf = (hdr_mysip*) msg;
		last_siph = sip_buf;
		
		if (sip_buf->ack==1) {
			if (sip_buf->method == 0) {
				cout << "+++sipA::recv INVITE+++\n";
				show_sipheader(sip_buf);
				set_contact(sip_buf);
				if(myID==88)
					send_temp_move_pkt();
				else
					send_200ok_pkt();
				
			}
			else if (sip_buf->method == 1) {
				cout << "+++sipA::recv 200OK+++\n";
				show_sipheader(sip_buf);
				invite_stop = 1;
				set_contact(sip_buf);
				send_mysip_data();
				send_ack_for_200ok_pkt();
				
			}
			else if (sip_buf->method == 2) {
				cout << "+++sipA::recv ACK+++\n";
				show_sipheader(sip_buf);
				m200ok_stop = 1;
				
			}
			else if (sip_buf->method == 3) {
				cout << "+++sipA::recv REGISTER+++\n";
				show_sipheader(sip_buf);
				cout << sip_buf->From_id <<"@" << sip_buf->From << endl;
				add_urllist(sip_buf->From_id, sip_buf->From, sip_buf->contact, 3);
				
			}
			else if (sip_buf->method == 4) {
				cout << "+++sipA::recv TEMPORY MOVE+++\n";
				show_sipheader(sip_buf);
				set_contact(sip_buf);
				send_invite_pkt();
				
			}
		}
		else {
			account_recv_pkt(sip_buf);
			
		}
			
	}
}

void mysipApp::set_scale(const hdr_mysip* mh_buf)
{
	scale_ = mh_buf->scale;
}

void mysipApp::account_recv_pkt(const hdr_mysip* mh_buf)
{
	double local_time = Scheduler::instance().clock();
	
	//	Calculate RTT 
	if (mh_buf->seq == 0) {
		init_recv_pkt_accounting();
		p_accnt.rtt = 2 * (local_time - mh_buf->time);
	}
	else
		p_accnt.rtt = 0.9 * p_accnt.rtt + 0.1 * 2 * (local_time - mh_buf->time);
	
	//printf("p_accnt.rtt %f\n",p_accnt.rtt);
	
	p_accnt.recv_pkts ++;
	p_accnt.lost_pkts += (mh_buf->seq - p_accnt.last_seq - 1);
	p_accnt.last_seq = mh_buf->seq;
	
	printf("p_accnt.recv_pkts %d p_accnt.lost_pkts %d p_accnt.last_seq %d p_accnt.rtt %f\n",
			p_accnt.recv_pkts, p_accnt.lost_pkts, p_accnt.last_seq, p_accnt.rtt);
}

void mysipApp::init_recv_pkt_accounting()
{
	p_accnt.last_seq = -1;
	p_accnt.last_scale = 0;
	p_accnt.lost_pkts = 0;
	p_accnt.recv_pkts = 0;
}

void mysipApp::send_ack_pkt(void)
{
	
	double local_time = Scheduler::instance().clock();
	printf("***%f send_mm_ack_pkt\n",local_time);
	
	adjust_scale();
	
	hdr_mysip ack_buf;
	ack_buf.ack = 1;
	ack_buf.time = local_time;
	ack_buf.nbytes = 40;
	ack_buf.scale = p_accnt.last_scale;
	agent_->sendmsg(ack_buf.nbytes, (char*)&ack_buf);
	
	ack_timer_.resched(p_accnt.rtt);
	
}

void mysipApp::adjust_scale(void)
{
	if(p_accnt.recv_pkts>0) {
		if(p_accnt.lost_pkts>0)
			p_accnt.last_scale = (int)(p_accnt.last_scale/2);
		else {
			p_accnt.last_scale++;
			if(p_accnt.last_scale>4) p_accnt.last_scale = 4;
		}
	}
	p_accnt.recv_pkts = 0;
	p_accnt.lost_pkts = 0;
	printf("p_accnt.last_scale %d \n",p_accnt.last_scale);
}

void mysipApp::dump_handoff()
{
	
}

void mysipApp::add_urllist(int url_id, int url, int addr, int port)
{
	cout << NOW << " mysip:addurllist" << endl;
	URLIPtable* n = new URLIPtable(url_id, url, addr, port);
	n->insert_URLIPtable(&url_head);
	dump_table();
	return;
}

URLIPtable* mysipApp::lookup_table(URLIPtable* node, int index_url_id, int index_url)
{
	if (node) {
		for ( ; node ; node=node->next_URLIPtable())
		{
			if (index_url_id == node->url_id) {
				cout << "\nFind a ID_URL record match!!" << " for node " << agent_->addr() << " at " << NOW << endl;
				cout << " URL \t IP \t Port \n"
							<< node->url_id << "@" << PRINTADDR(node->url) << "\t"
							<< PRINTADDR(node->IP) << "\t"
							<< node->Port << "\t"
							<< "\n";
				return node;
				break;
			}
		}
	}
	return 0;
}

void mysipApp::dump_table()
{
	dump_urliptable(headu(url_head), "mysip:URL Cache");
}

void mysipApp::dump_urliptable(URLIPtable* node, char* txt)
{
	if (node) {
		printf("%d@%s URL vs IP table\n", myID, PRINTADDR(myURL));
		cout << "URL \t IP \t Port \n";
		
		for ( ; node; node=node->next_URLIPtable() )
		{
			cout << node->url_id << "@" << PRINTADDR(node->url) << "\t"
						<< PRINTADDR(node->IP) << "\t"
						<< node->Port << "\t"
						<< "\n";
		}
		cout << "\n";
	}
}

void mysipApp::set_contact(const hdr_mysip *sip_buf)
{
	contact_dst_addr = sip_buf->contact;
}
