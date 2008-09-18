//  
// Author:    Jae Chung  
// File:      mm-app.cc
// Written:   07/17/99 (for ns-2.1b4a)  
// Modifed:   10/14/01 (for ns-2.1b8a)  
//   

#include <iostream>
#include "random.h"
#include "mysip-app.h"

// mysipApp OTcl linkage class
static class mysipAppClass : public TclClass {
 public:
  mysipAppClass() : TclClass("Application/mysipApp") {}
  TclObject* create(int, const char*const*) {
    return (new mysipApp);
  }
} class_app_sip;


// Constructor (also initialize instances of timers)
mysipApp::mysipApp() : running_(0), snd_timer_(this), ack_timer_(this),ok_timer_(this), invite_timer_(this),CSeq_(1),invite_stop(0),m200ok_stop(0),ack_stop(0),error_recover_num(0),last_reinvite_time(0)

{
  //bind("myID_", &myID);
  //bind("myURL_", &myURL);
  bind_bw("rate0_", &rate[0]);
  bind_bw("rate1_", &rate[1]);
  bind_bw("rate2_", &rate[2]);
  bind_bw("rate3_", &rate[3]);
  bind_bw("rate4_", &rate[4]);
  bind("pktsize_", &pktsize_);
  bind_bool("random_", &random_);

  //0401  be very careful for add_urllist
  LIST_INIT(&url_head);
}


// OTcl command interpreter
int mysipApp::command(int argc, const char*const* argv)
{
  Tcl& tcl = Tcl::instance();
  //0402
  if (argc == 2 )
  {
      
	  if ( strcmp(argv[1], "dump_handoff_info") == 0 )
	  {
		  dump_handoff();
		  return(TCL_OK);
	  }
	  if ( strcmp(argv[1], "measure_handoff") == 0 )
	  {
		  measure_handoff();
		  return(TCL_OK);
	  }

  }

  if (argc == 3) 
  {
	  if ( strcmp(argv[1], "set_addr") == 0 )
	  	 {
	  		  agent_->new_addr =STR2ADDR(argv[2]);
	  		  agent_->addr()=STR2ADDR(argv[2]);
	  		  printf("%s %d\n",argv[2],agent_->new_addr);
	  		  return(TCL_OK);
	  	 }
	  
		if ( strcmp(argv[1], "registration") == 0 )
		{
						agent_->new_addr =STR2ADDR(argv[2]);
						agent_->addr()=STR2ADDR(argv[2]);
						printf("%s %d\n",argv[2],agent_->new_addr);
//						tcl.evalf("%s set agent_addr_ %s",agent_->name(),argv[2]);
//						printf("%s %d\n",argv[2],agent_->addr());
		      send_register_pkt();
		      tcl.resultf("%d@%s do registration\n",myID,PRINTADDR(myURL));
		      return(TCL_OK);
		}
	if (strcmp(argv[1], "attach-agent") == 0)
	{
	      agent_ = (Agent*) TclObject::lookup(argv[2]);
	      if (agent_ == 0) {
		tcl.resultf("no such agent %s", argv[2]);
		return(TCL_ERROR);
	      }

	      // Make sure the underlying agent support MM
	      if(agent_->supportMM()) {
		agent_->enableMM();
	      }
	      else {
		tcl.resultf("agent \"%s\" does not support MM Application", argv[2]);
		return(TCL_ERROR);
	      }
	      
	      agent_->attachApp(this);
	      return(TCL_OK);
	}
  }
  //0401
  if(argc == 4) 
  {
	if (strcmp(argv[1], "myID_URL") == 0 )
	{
	    myID = atoi(argv[2]);
	    myURL = STR2ADDR(argv[3]);
	    myID_URL();
	    tcl.resultf("myURL is %d@%s\n",myID,PRINTADDR(myURL));
	    return(TCL_OK);
	}
	if (strcmp(argv[1], "send_invite") == 0 )
	{
	    toID = atoi(argv[2]);
	    toURL = STR2ADDR(argv[3]);
	    start();
	    tcl.resultf("%d@%s toURL is %d@%s\n",myID,PRINTADDR(myURL),toID,PRINTADDR(toURL));
	    return(TCL_OK);
	}
//	if (strcmp(argv[1], "send_re_invite") == 0 )
//	{
//	    toID = atoi(argv[2]);
//	    toURL = STR2ADDR(argv[3]);
//	    running_ = 1;
//	    contact_dst_addr= toURL;
//	    send_invite_pkt();
//	    tcl.resultf("%d@%s toURL is %d@%s\n",myID,PRINTADDR(myURL),toID,PRINTADDR(toURL));
//	    return(TCL_OK);
//	}
	if (strcmp(argv[1], "log_file") == 0 )
	{
	    strcpy(filename,argv[2]);
	    strcpy(filename_sec,argv[3]);
	    return(TCL_OK);
	}

  }
  if(argc == 5) 
  {	
	if (strcmp(argv[1], "add_URL_record") == 0 )
	{
	    int xurl_id = atoi(argv[2]);
	    int xurl = STR2ADDR(argv[3]);
	    int xip = STR2ADDR(argv[4]);
	    add_urllist( xurl_id, xurl, xip, 3 );
	    return(TCL_OK);
	}
  }

  return (Application::command(argc, argv));
}

void mysipApp::myID_URL()
{
    printf("myID_URL() say: myURL is %d@%d\n",myID,myURL);
}
    
    

void mysipApp::init()
{
  scale_ = 0; // Start at minimum rate
  seq_ = 0;   // MM sequence number (start from 0)
  agent_->new_addr = agent_->addr();
  printf("mysipA::init new_addr=%d addr=%d\n",agent_->new_addr,agent_->addr());
  interval_ = (double)(pktsize_ << 3)/(double)rate[scale_];
}


void mysipApp::start()
{
  init();
  cout << "myURL is " << myID <<"@"<< PRINTADDR(myURL) <<endl;
  running_ = 1;
  contact_dst_addr= toURL;
//  send_mysip_data();
  send_invite_pkt();
}


void mysipApp::stop()
{
  running_ = 0;
}


// Send application data packet
void mysipApp::send_mysip_data()
{
  cout << NOW << " mysipApp::send_mysip_data c_dst_addr " << contact_dst_addr <<endl;
  hdr_mysip sip_buf;

  if (running_) {
    // the below info is passed to Udpmysip agent, which will write it 
    // to MM header after packet creation.
    sip_buf.ack = 0;            // This is a MM packet
    sip_buf.seq = seq_++;         // MM sequece number
    sip_buf.nbytes = pktsize_;  // Size of MM packet (NOT UDP packet size)
    sip_buf.time = Scheduler::instance().clock(); // Current time
    sip_buf.scale = scale_;                       // Current scale value
    sip_buf.method = 1;
    sip_buf.requestURL = contact_dst_addr;
    sip_buf.From = agent_->addr();
    sip_buf.To = agent_->daddr();
    sip_buf.CSeq = 1;
    sip_buf.contact = agent_->get_new_addr();
    //show_sipheader((const char*) &sip_buf);
    agent_->sendmsg(pktsize_, (char*) &sip_buf);  // send to UDP

//   snd_timer_.resched(0.01);
/*     
 *     FILE *op;
 *     op = fopen(filename,"a");
 *     fprintf(op,"%d %lf\n",sip_buf.seq,sip_buf.time);
 *     fclose(op);
 */

    // Reschedule the send_pkt timer
/*     double next_time_ = next_snd_time();
 *     if(next_time_ > 0){
 *         cout << "send_mysip_data next_time="<<next_time_ <<endl;
 *     	snd_timer_.resched(next_time_);
 *     	//invite_timer_.resched(next_time_);
 *  }
 */
  }
}


// Schedule next data packet transmission time
double mysipApp::next_snd_time()
{
  // Recompute interval in case rate or size chages
  interval_ = (double)(pktsize_ << 3)/(double)rate[scale_];
  double next_time_ = interval_;
  if(random_) 
    next_time_ += interval_ * Random::uniform(-0.5, 0.5);
  return next_time_;
}


// Receive message from underlying agent
void mysipApp::recv_msg(int nbytes, const char *msg)
{
  cout<< NOW <<" mysipApp::recv_msg \n"; 
  if(msg) {
    hdr_mysip* sip_buf = (hdr_mysip*) msg;
    last_siph = sip_buf;

    if ( sip_buf->ack == 1 )
    {
	if(sip_buf->method == 0) {
	    // If received packet is 200ok packet
	    cout << "***********sipA::recv INVITE**************\n";
	    show_sipheader(sip_buf);
	    set_contact(sip_buf);
            if(myID == 88 )
		send_temp_move_pkt();
	    else
		send_200ok_pkt();
/* 	    if(NOW > 40.0 && ( NOW > (last_reinvite_time + 20.0)))
 * 	    {
 * 		error_recover_time[error_recover_num++] = NOW - sip_buf->time;
 * 		last_reinvite_time = NOW;
 * 	    }
 */

	}
	else if(sip_buf->method == 1) {
	    // If received packet is 200ok packet
	   // set_scale(sip_buf);
	   cout << "***********sipA::recv 200OK**************\n";
	   show_sipheader(sip_buf);
	   invite_stop = 1;
	   set_contact(sip_buf);
	   send_mysip_data();
	   send_ack_for_200ok_pkt();
/* 	   if(NOW > 40.0 && ( NOW > (last_reinvite_time + 20.0)))
 * 	  {
 * 		error_recover_time[error_recover_num++] = NOW - sip_buf->time;
 * 		last_reinvite_time = NOW;
 * 	   }
 */

	}
	else if(sip_buf->method == 2) {
	    cout << "***********sipA::recv ACK**************\n";
	    show_sipheader(sip_buf);
	    m200ok_stop = 1;
	    // If received packet is ack packet

	}
	else if(sip_buf->method == 3) {
	    cout << "***********sipA::recv REGISTER**************\n";
	    show_sipheader(sip_buf);
	    cout << sip_buf->From_id <<"@"<< sip_buf->From << endl;
	    add_urllist(sip_buf->From_id,sip_buf->From,sip_buf->contact,3);
	    //m200ok_stop = 1;
	    // If received packet is ack packet
	}
	else if(sip_buf->method == 4) {
	    cout << "***********sipA::recv TEMPORY MOVE**************\n";
	    show_sipheader(sip_buf);
	    set_contact(sip_buf);
            send_invite_pkt();
	    //m200ok_stop = 1;
	    // If received packet is ack packet
	}
    }
    else {
      // If received packet is invite packet
      account_recv_pkt(sip_buf);
      ack_stop = 1;
      /*if(sip_buf->seq == 0) {
	 send_200ok_pkt();
      }*/
    }
  }
}

void mysipApp::show_sipheader(struct hdr_mysip* msg)
{	
   cout<<"sipH method: "<< msg->method << "\t"
       <<" reqURL: " << msg->requestURL_id <<"@"<< PRINTADDR(msg->requestURL) << endl
       <<" From: " << msg->From_id <<"@"<< PRINTADDR(msg->From) << endl
       <<" To: " << msg->To_id <<"@"<< PRINTADDR(msg->To)  << endl
       <<" contact: "<< msg->contact_id <<"@"<< PRINTADDR(msg->contact) << endl  
       <<" CSeq: "<< msg->CSeq << endl  
       <<" cip: "<< PRINTADDR(msg->cip) << endl
       <<" cport: "<< msg->cport << endl
       << endl;
}

// Sender sets its scale to what reciver notifies
void mysipApp::set_scale(const hdr_mysip *sip_buf)
{ 
  scale_ = sip_buf->scale;
}


void mysipApp::set_contact(const hdr_mysip *sip_buf)
{ 
  contact_dst_addr = sip_buf->contact;
}

void mysipApp::account_recv_pkt(const hdr_mysip *sip_buf)
{ 
  double local_time = Scheduler::instance().clock();

  // Calculate RTT
  if(sip_buf->seq == 0) {
    init_recv_pkt_accounting();
    p_accnt.rtt = 2*(local_time - sip_buf->time);
  }
  else
    p_accnt.rtt = 0.9 * p_accnt.rtt + 0.1 * 2*(local_time - sip_buf->time); 

  // Count Received packets and Calculate Packet Loss
  p_accnt.recv_pkts ++;
  p_accnt.lost_pkts = (sip_buf->seq - p_accnt.last_seq - 1);
  p_accnt.total_lost_pkts += p_accnt.lost_pkts;
  FILE *op;
  op = fopen(filename,"a");
  // seq sendtime arrivaltime rtt lost 
  if(p_accnt.lost_pkts > 2 && handoffnum < 30)
  {
      fprintf(op,"%d %lf %lf %lf %d +\n",sip_buf->seq,sip_buf->time,local_time,p_accnt.rtt, p_accnt.lost_pkts);
      handofftime[handoffnum++] = local_time - p_accnt.last_arrival_time;
      //invite_timer_.resched(0.05);
  }
  else
      fprintf(op,"%d %lf %lf %lf %d\n",sip_buf->seq,sip_buf->time,local_time,p_accnt.rtt, p_accnt.lost_pkts);

  fclose(op);

  p_accnt.last_seq = sip_buf->seq;
  p_accnt.last_arrival_time = NOW;
}

void mysipApp::dump_handoff()
{
  FILE *op;
  op = fopen(filename_sec,"a");
  for(int i=0; i < handoffnum; i++)
      fprintf(op,"%lf\n",handofftime[i]);
  fprintf(op,"%d\n",p_accnt.total_lost_pkts);
  for( int j=0; j < error_recover_num; j++)
  {
      fprintf(op,"%lf\n",error_recover_time[j]);
  }
  fprintf(op,"123456789\n");
  fclose(op);
}

void mysipApp::measure_handoff()
{
  FILE *op,*opw;
  int count = 0,i=0,j=0,k=0;
  double tp;
  int error;
  int lost;
  double temp[error_recover_num];
  double temp1[error_recover_num];
  double temp2[error_recover_num];
  double average=0.0;
  op = fopen(filename_sec,"r");
  opw = fopen(filename,"w");
  while( fscanf(op,"%lf",&tp) > 0 )
  {
        if( count < error_recover_num )
	    temp[i++] = tp;
	else if( count == error_recover_num )
	    lost = (int)tp;
	else if( count < 2*error_recover_num + 1)
	    temp1[j++] = tp;
	else if( count == 2*error_recover_num + 1 )
	    if( (int)tp != 123456789 )
		    error = 1;
	else if( count == 2*error_recover_num + 2 )
	    if( (int)tp != 0 )
		    error = 2;
	else if( count < 3*error_recover_num + 3 )
	{
	    temp2[k] = tp + temp[k] + temp1[k];
	    average += temp2[k];
	    k++;
	}
	else if( count == 3*error_recover_num + 3 )
	    if( (int)tp != 123456789 )
		error = 3;
	else
	    error = 4;	
	
	count++;
  }
  for( i = 0 ; i < error_recover_num ; i++)
      fprintf(opw,"%lf\n",temp2[i]);
  fprintf(opw,"average == %lf\n", average);
  fprintf(opw,"error == %d\n", error);
  fclose(op);
}


void mysipApp::init_recv_pkt_accounting()
{
   FILE *op;
   op = fopen(filename,"w");
   fclose(op);
   op = fopen(filename_sec,"w");
   fclose(op);
   p_accnt.last_seq = -1;
   p_accnt.last_scale = 0; 
   p_accnt.lost_pkts = 0;
   p_accnt.recv_pkts = 0;
   handoffnum = 0;
   p_accnt.last_arrival_time = 0.0;

}


void mysipApp::send_invite_pkt(void)
{
	printf("mysipapp addr %s\n",PRINTADDR(agent_->addr()));
	printf("mysipapp daddr %s\n",PRINTADDR(agent_->daddr()));
	
  cout << NOW << " sipA::invite c_dst_addr "<< contact_dst_addr << endl;


  // send invite message
  hdr_mysip invite_buf;
  invite_buf.ack = 1;  // this pinviteet is invite pinviteet
  invite_buf.time = NOW;
  invite_buf.seq = -2;         // MM sequece number
  invite_buf.nbytes = 120;  // invite pinviteet size is 40 Bytes
  invite_buf.scale = p_accnt.last_scale;
  invite_buf.method = 0;
  invite_buf.From_id = myID;
  invite_buf.From = myURL;
  invite_buf.To_id = toID;
  invite_buf.To = toURL;
  if( CSeq_ ==1 )
  {
      agent_->new_addr = agent_->addr();
      cout << "mysipA::send invite new_addr=" << PRINTADDR(agent_->new_addr) << " addr=" << PRINTADDR(agent_->addr()) << endl;
  }
  invite_buf.CSeq = CSeq_++;
  invite_buf.contact_id = myID;
  invite_buf.contact = agent_->get_new_addr();
  invite_buf.requestURL_id = toID;
  invite_buf.requestURL = contact_dst_addr;
  invite_buf.cip = agent_->get_new_addr();
  invite_buf.cport = 3;
  show_sipheader(&invite_buf);
  agent_->sendmsg(invite_buf.nbytes, (char*) &invite_buf);

  // schedul next invite time
  cout << "sipA::invite next_time:"<<p_accnt.rtt <<" invite_stop:" << invite_stop << endl;
  if(!invite_stop)
      invite_timer_.resched(2);
}



void mysipApp::send_200ok_pkt(void)
{
  cout << NOW << " sipA::200ok c_dst_addr "<< contact_dst_addr << endl;

  adjust_scale();

  // send ok message
  hdr_mysip ok_buf;
  memcpy(&ok_buf,last_siph,sizeof(struct hdr_mysip));
  ok_buf.ack = 1;  // this poket is ok poket
  ok_buf.time = NOW;
  ok_buf.seq = -2;         // MM sequece number
  ok_buf.nbytes = 80;  // ok poket size is 40 Bytes
  ok_buf.scale = p_accnt.last_scale;
  ok_buf.method = 1;
  ok_buf.contact = agent_->get_new_addr();
  if( CSeq_ ==1 )
  {
      agent_->new_addr = agent_->addr();
      printf("mysipA::send 200ok new_addr=%d addr=%d\n",agent_->new_addr,agent_->addr());

  }
  ok_buf.CSeq = CSeq_++;
  ok_buf.contact_id = myID;
  ok_buf.contact = agent_->new_addr;
  ok_buf.requestURL = contact_dst_addr;
  ok_buf.cip = agent_->new_addr;
  ok_buf.cport = 3;
  show_sipheader(&ok_buf);
  agent_->sendmsg(ok_buf.nbytes, (char*) &ok_buf);

  // schedul next ok time
  cout << "send_200ok_pkt next_time="<<p_accnt.rtt <<endl;
  //ok_timer_.resched(p_accnt.rtt);
}


void mysipApp::send_ack_for_200ok_pkt(void)
{

  cout << NOW << " sipA::ack c_dst_addr " << contact_dst_addr << endl;

  // send ack message for 200ok
  hdr_mysip ack_buf;
  memcpy(&ack_buf,last_siph,sizeof(struct hdr_mysip));
  ack_buf.ack = 1;  // this packet is ack_200ok packet
  ack_buf.time = NOW;
  ack_buf.seq = -2;         // MM sequece number
  ack_buf.nbytes = 40;  // Ack packet size is 40 Bytes
  ack_buf.scale = p_accnt.last_scale;
  ack_buf.method = 2;
  ack_buf.From = agent_->addr();
  ack_buf.To = agent_->daddr();
  ack_buf.CSeq = CSeq_++;
  ack_buf.contact=agent_->get_new_addr();
  ack_buf.requestURL = contact_dst_addr;
  agent_->sendmsg(ack_buf.nbytes, (char*) &ack_buf);

  // schedul next ACK time
  //cout << "send_ack_for_200ok_pkt next_time="<<p_accnt.rtt <<endl;
  //snd_timer_.resched(0.5);
  //ack_timer_.resched(0.5);

}

void mysipApp::adjust_scale(void)
{
  if(p_accnt.recv_pkts > 0) {
    if(p_accnt.lost_pkts > 0)
      p_accnt.last_scale = (int)(p_accnt.last_scale / 2);
    else {
      p_accnt.last_scale++;
      if(p_accnt.last_scale > 4) p_accnt.last_scale = 4;
    }
  }
  p_accnt.recv_pkts = 0;
  p_accnt.lost_pkts = 0;
}

void InviteTimer::expire( Event* e)
{
    printf("InviteTimer expire ....\n");
    a_->send_invite_pkt();
}

void SendTimer::expire(Event* e){
    printf("mysipApp SendTimer expire call send_mysip_data\n");
    t_->send_mysip_data();
}
void AckTimer::expire(Event* e){
    printf("mysipApp AckTimer expire call send_ack_for_200ok_pkt\n");
    t_->send_ack_for_200ok_pkt();
}
void OKTimer::expire(Event* e){
    printf("mysipApp AckTimer expire call send_200ok_pkt\n");
    t_->send_200ok_pkt();
}


// for URL_IP_table use 0401


void mysipApp::add_urllist(int url_id, int url, int addr, int port)
{
	cout << "\n" << NOW << " mysip:addurllist" << endl; 
	URLIPtable* n = new URLIPtable(url_id,url,addr,port);
	n->insert_URLIPtable(&url_head);
        dump_table();
	return;
}

URLIPtable* mysipApp::lookup_table(URLIPtable* node, int index_url_id, int index_url)
{
	if ( node )
	{

		for ( ; node ; node=node->next_URLIPtable() )
		{
		    if ( index_url_id == node->url_id )
		    {
			cout <<"\nFind a ID_URL record match!! "<< " for node "<< agent_->addr() <<" at "<< NOW <<"\n";

			cout << "| URL\tIP\tPort |\n"
			     << node->url_id<<"@"<<PRINTADDR(node->url) << "\t"
			     << PRINTADDR(node->IP) << "\t"
			     << node->Port << "\t"
			     <<"\n";

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
	if ( node )
	{
	//	cout <<"\n|"<< txt << " for node "<< agent_->addr() <<" at "<< NOW <<"\n";
		printf("%d@%s URL vs IP table\n",myID,PRINTADDR(myURL));
		cout <<"|URL \t IP \t Port\n";

		for ( ; node ; node=node->next_URLIPtable() )
		{
		    cout << node->url_id<<"@"<<PRINTADDR(node->url) << "\t"
			 << PRINTADDR(node->IP) << "\t"
			 << node->Port << "\t"
			 <<"\n";
		}

		cout << "\n";
	}
}

//0403 registration to SIP server
void mysipApp::send_register_pkt(void)
{
	printf("mysipapp addr %s\n",PRINTADDR(agent_->addr()));
	printf("mysipapp daddr %s\n",PRINTADDR(agent_->daddr()));
	
  cout << NOW << " sipA::register c_dst_addr "<< contact_dst_addr << endl;


  // send register message
  hdr_mysip register_buf;
  register_buf.ack = 1;  // this pregisteret is register pregisteret
  register_buf.time = NOW;
  register_buf.seq = -2;         // MM sequece number
  register_buf.nbytes = 200;  // register pregisteret size is 40 Bytes
  register_buf.scale = p_accnt.last_scale;
  register_buf.method = 3;
  register_buf.requestURL = myURL;
  register_buf.requestURL_id = 88;
  register_buf.From_id = myID;
  register_buf.From = myURL;
  register_buf.To_id = myID;
  register_buf.To = myURL;
  if( CSeq_ ==1 )
  {
      agent_->new_addr = agent_->addr();
      printf("mysipA::%d@%s send register new_addr=%s addr=%s\n",myID,PRINTADDR(myURL),PRINTADDR(agent_->new_addr),PRINTADDR(agent_->addr()));

  }
  register_buf.CSeq = CSeq_++;
  register_buf.contact_id = myID;
  register_buf.contact = agent_->get_new_addr();
  register_buf.cport = 3;
  struct hdr_mysip* temp = &register_buf;
  show_sipheader(temp);
  agent_->sendmsg(register_buf.nbytes, (char*) &register_buf);
  // schedul next register time
  //cout << "sipA::register next_time:"<<p_accnt.rtt <<" register_stop:" << register_stop << endl;
  //if(!register_stop)
    //  register_timer_.resched(2);
}

//0404 SIP server send tempority moving
void mysipApp::send_temp_move_pkt(void)
{

  cout << NOW << " sipA::send temp_move c_dst_addr "<< contact_dst_addr << endl;

  // send tempmove message
  struct hdr_mysip tempmove_buf;
  memcpy(&tempmove_buf,last_siph,sizeof(struct hdr_mysip));
  tempmove_buf.ack = 1;  // this ptempmoveet is tempmove ptempmoveet
  tempmove_buf.time = NOW;
  tempmove_buf.seq = -2;         // MM sequece number
  tempmove_buf.nbytes = 300;  // tempmove ptempmoveet size is 40 Bytes
  tempmove_buf.scale = p_accnt.last_scale;
  tempmove_buf.method = 4;
  tempmove_buf.requestURL = last_siph->contact;
  tempmove_buf.requestURL_id = last_siph->contact_id;
  URLIPtable* urlentry = lookup_table( headu(url_head),last_siph->requestURL_id, last_siph->requestURL);
  if((myID = 88) && urlentry)
  {
      tempmove_buf.contact = urlentry->IP;
     
      show_sipheader(&tempmove_buf);
      agent_->sendmsg(tempmove_buf.nbytes, (char*) &tempmove_buf);
      // schedul next tempmove time
      //cout << "sipA::tempmove next_time:"<<p_accnt.rtt <<" tempmove_stop:" << tempmove_stop << endl;
      //if(!tempmove_stop)
	//  tempmove_timer_.resched(2);
  }
}

