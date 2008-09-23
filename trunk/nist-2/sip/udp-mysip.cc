//
// Author:    Jae Chung
// File:      udp-mm.cc
// Written:   07/17/99 (for ns-2.1b4a)
// Modifed:   10/14/01 (for ns-2.1b8a)
//

#include "udp-mysip.h"
#include "rtp.h"
#include "random.h"
#include <string.h>


int hdr_mysip::offset_;

// Mulitmedia Header Class 
static class MysipHeaderClass : public PacketHeaderClass {
public:
	MysipHeaderClass() : PacketHeaderClass("PacketHeader/MYSIP",
						    sizeof(hdr_mysip)) {
		bind_offset(&hdr_mysip::offset_);
	}
} class_mysiphdr;


// UdpmysipAgent OTcl linkage class
static class UdpmysipAgentClass : public TclClass {
public:
	UdpmysipAgentClass() : TclClass("Agent/UDP/Udpmysip") {}
	TclObject* create(int, const char*const*) {
		return (new UdpmysipAgent());
	}
} class_Udpmysip_agent;


// Constructor (with no arg)
UdpmysipAgent::UdpmysipAgent() : UdpAgent(), mipv6_(NULL), node_type_(SIP_CN), flag(1)//, mysipapp_(NULL)
{
	LIST_INIT(&siplist_head_);
	bind("packetSize_",&size_);
	support_mm_ = 0;
	session_run = 0;
	//new_addr = 0;
//	printf("UdpmysipA() new_addr=%d\n",new_addr);
	asm_info.seq = -1;
}

int UdpmysipAgent::command(int argc, const char*const* argv) {
	if (argc==3) {
		if (strcmp(argv[1], "set-mipv6")==0) {
			mipv6_= (MIPV6Agent *) TclObject::lookup(argv[2]);
			if(mipv6_==0)
				return TCL_ERROR;
			return TCL_OK;
		}
//		if (strcmp(argv[1], "set-mysipapp")==0) {
//			mysipapp_ = (mysipApp *) TclObject::lookup(argv[2]);
//			if(mysipapp_==0)
//				return TCL_ERROR;
//			return TCL_OK;
//		}
		if (strcmp(argv[1], "set-node-type")==0)
		{
			switch (atoi(argv[2]))
			{
			case SIP_MN:
				node_type_ = SIP_MN;
				break;
			case SIP_MN_HA:
				node_type_ =  SIP_MN_HA;
				break;
			case SIP_MR:
				node_type_ = SIP_MR;
				break;
			case SIP_MR_HA:
				node_type_ = SIP_MR_HA;
				break;
			case SIP_CN:
				node_type_ = SIP_CN;
				break;
			default:
				debug("no match type\n");
				return TCL_ERROR;
				break;
			}
			debug("node_type_ %d\n",node_type_);
			return TCL_OK;
		}
	}
	if (argc==7) 
	{
		if (strcmp(argv[1], "set-sip-mn")==0) 
		{
			int add_id_ = atoi(argv[2]);
			int add_ = Address::instance().str2addr(argv[3]);
			int url_id_ = atoi(argv[4]);
			int url_ = Address::instance().str2addr(argv[5]);
			NEMOAgent *eface_agent_ = (NEMOAgent *)TclObject::lookup(argv[6]);		// external interface nemo agent
			if (eface_agent_==NULL)
				return TCL_ERROR;
			SIPEntry* sip = new SIPEntry(SIP_MN_HA);
			sip->mn_init_entry(add_id_, add_, url_id_, url_, eface_agent_);
			sip->insert_entry(&siplist_head_);
			dump();
			return TCL_OK;
		}
	}
	if (argc==9)
	{
		if (strcmp(argv[1], "set-sip-mr")==0) 
		{
			int add_id_ = atoi(argv[2]);
			int add_ = Address::instance().str2addr(argv[3]);
			int url_id_ = atoi(argv[4]);
			int url_ = Address::instance().str2addr(argv[5]);
			int nemo_prefix_ = Address::instance().str2addr(argv[6]);							//	nemo prefix
			NEMOAgent *eface_agent_ = (NEMOAgent *)TclObject::lookup(argv[7]);		// external interface nemo agent
			NEMOAgent *iface_agent_ = (NEMOAgent *)TclObject::lookup(argv[8]);		// internal interface nemo agent
			if (eface_agent_==NULL && iface_agent_==NULL)
				return TCL_ERROR;
			SIPEntry* sip = new SIPEntry(SIP_MR_HA);
			sip->mr_init_entry(add_id_, add_, url_id_, url_, nemo_prefix_, eface_agent_, iface_agent_);
			sip->insert_entry(&siplist_head_);
			dump();
			return TCL_OK;
		}
	}
	return (UdpAgent::command(argc, argv));
}

UdpmysipAgent::UdpmysipAgent(packet_t type) : UdpAgent(type)
{
	support_mm_ = 0;
	asm_info.seq = -1;
}


// Add Support of Multimedia Application to UdpAgent::sendmsg
void UdpmysipAgent::sendmsg(int nbytes, const char* flags)
{
	printf("udpMysipAgent send!\nsize_ %d nbytes %d\n", size_, nbytes);
	Packet *p;
	int n, remain;
 
	if (size_) {
		n = (nbytes/size_ + (nbytes%size_ ? 1 : 0));
		//printf("nbytes=%d size_=%d n=%d\n",nbytes,size_,n);
		remain = nbytes%size_;
	}
	else
		printf("Error: Udpmysip size = 0\n");

	if (nbytes == -1) {
		printf("Error:  sendmsg() for Udpmysip should not be -1\n");
		return;
	}
	double local_time =Scheduler::instance().clock();
	while (n-- > 0) {
		p = allocpkt();
		if(n==0 && remain>0) hdr_cmn::access(p)->size() = remain;
		else hdr_cmn::access(p)->size() = size_;
		hdr_rtp* rh = hdr_rtp::access(p);
		hdr_ip* ih = hdr_ip::access(p);
		rh->flags() = 0;
		rh->seqno() = ++seqno_;
		hdr_cmn::access(p)->timestamp() = (u_int32_t)(SAMPLERATE*local_time);
		// to eliminate recv to use MM fields for non MM packets
		hdr_mysip* siph = hdr_mysip::access(p);
		siph->ack = 0;
		siph->seq = 0;
		siph->nbytes = 0;
		siph->time = 0;
		siph->scale = 0;
		// mm udp packets are distinguished by setting the ip
		// priority bit to 15 (Max Priority).
		
		if(support_mm_) {
			ih->prio_ = 15;
			memcpy(siph, flags, sizeof(hdr_mysip));
			ih->daddr() = Address::instance().get_nodeaddr(siph->requestURL);
			ih->saddr() = Address::instance().get_nodeaddr(siph->contact);
			if(!siph->ack)
			{
			    ih->dport() = Address::instance().get_nodeaddr(cport_);
			}
			else
			    ih->dport() = Address::instance().get_nodeaddr(siph->cport);
//			ih->dport()=0;
//			ih->sport()=0;
		//	printf("UdpmysipAgent\n\tsaddr() %s:%d daddr()=%s:%d\n\tnew_addr=%s\n",PRINTADDR(ih->saddr()),ih->sport(),PRINTADDR(ih->daddr()),ih->dport(),PRINTADDR(new_addr));
			/*if(flags){
			// MM Seq Num is passed as flags
				memcpy(siph, flags, sizeof(hdr_mysip));
			        ih->daddr() = Address::instance().get_nodeaddr(siph->requestURL);
			        printf("ih->daddr()=%d rURL=%d\n",ih->daddr(),siph->requestURL);
			}*/
			
		}
		// add "beginning of talkspurt" labels (tcl/ex/test-rcvr.tcl)
		if (flags && (0 ==strcmp(flags, "NEW_BURST")))
			rh->flags() |= RTP_M;
		
		//----------------sem start------------------//
		hdr_ip* iph = HDR_IP(p);
		hdr_mysip *mh= HDR_MYSIP(p);
		if(node_type_==SIP_MN) {
			
			int prefix = iph->saddr() & 0xFFFFF800;
			debug("prefix=%s\n",Address::instance().print_nodeaddr(prefix));
			
			iph->daddr() = prefix;
			iph->dport() = port();
			
			SIPEntry* bu = get_entry_by_type(SIP_MN_HA);
			assert(bu!=NULL);
			Packet* p_tunnel = p->copy();
			bu->eface()->send(p_tunnel,0);
			debug("At %f UdpmysipAgent MN in %s send tunnel packet to %s\n", 
					NOW, MYNUM, Address::instance().print_nodeaddr(iph->daddr()));
			flag=0;
			
		}else if(node_type_==SIP_MR_HA) {
			if(mh->method==5)
			{
				printf("SIP_MR_HA register");
				
			}
		}
		//----------------sem end------------------//
		
		printf("UdpmysipAgent\n\tsaddr() %s:%d daddr()=%s:%d\n\tnew_addr=%s\n",PRINTADDR(ih->saddr()),ih->sport(),PRINTADDR(ih->daddr()),ih->dport(),PRINTADDR(new_addr));
				
		if(flag==1){
			if(mipv6_!=0 )
			{
				printf("mipv6 enable\n");
				mipv6_->tunneling(p);
			}
			else	
				target_->recv(p);
		}
		flag=1;
		
	}
	idle();
}


// Support Packet Re-Assembly and Multimedia Application
void UdpmysipAgent::recv(Packet* p, Handler*)
{
	debug("At %f UdpmysipAgent Agent in %s recv packet \n", NOW, MYNUM);
	hdr_ip* ih = hdr_ip::access(p);
	int bytes_to_deliver = hdr_cmn::access(p)->size();
	//printf("UdpmysipAgent recv ih->prio_ ==%d\n",ih->prio_);
	
	//----------------sem start------------------//
	hdr_ip* iph = HDR_IP(p);
	hdr_cmn *hdrc= HDR_CMN(p);
	hdr_mysip *mh= HDR_MYSIP(p);
	if(node_type_ == SIP_MR) {
		if(mh->method==5)
		{
			//	mn has ha, register myself and change mn contact to mr-ha contact
			//	mn has no ha, register myself
			SIPEntry* bu = get_entry_by_url_id(mh->contact_id);
			if(!bu)
			{
				registration(p,SIP_MN);
				dump();
			}
			
			int prefix = iph->saddr() & 0xFFFFF800;
							
			bu= get_entry_by_prefix(prefix);

			assert(bu!=NULL);

			//	change change mn contact to mr-ha contact
			mh->contact=bu->url();
			mh->contact_id=bu->url_id();
			
			show_sipheader(p);
			
			//	chage daddr to mn ha
			//	we use header tunnel in mip
			iph->daddr() = mh->requestURL;
				
			Packet* p_tunnel = p->copy();

			bu->eface()->send(p_tunnel,0);
			//bu->eface()->send_bu_ha(p_tunnel);
			//bu->eface()->recv(p_tunnel,0);
			
			debug("At %f UdpmysipAgent MR in %s tunnel register packet to %s\n", 
					NOW, MYNUM, Address::instance().print_nodeaddr(iph->daddr()));
						
		}
		else if(mh->method==0 || mh->method==7)
		{
			debug("At %f UdpmysipAgent MR in %s recv packet\n", NOW, MYNUM);
			
			SIPEntry* bu = get_entry_by_url_id(mh->To_id);
			
			assert(bu!=NULL);
			
			int prefix = bu->con() & 0xFFFFF800;
			
			debug("prefix=%s\n",Address::instance().print_nodeaddr(prefix));
			
			iph->daddr() = bu->con();
			
			Packet* p_tunnel = p->copy();
			get_iface_agent_by_prefix(prefix)->send(p_tunnel,0);
			
			debug("At %f UdpmysipAgent MR in %s send packet to %s\n", 
								NOW, MYNUM, Address::instance().print_nodeaddr(iph->daddr()));
			//send_temp_move_pkt(p);
			//send_temp_move_nemo(p);
			ih->prio_ = 14;
		}
		else if(mh->method==1)
		{
			//SIPEntry* bu = get_entry_by_url_id(iph->daddr());
			//SIPEntry* bu = get_entry_by_type(SIP_MR_HA);
			int prefix = iph->saddr() & 0xFFFFF800;
			
			SIPEntry* bu = get_entry_by_prefix(prefix);
			assert(bu!=NULL);
			iph->daddr() = mh->requestURL;
			mh->contact= bu->con();
			Packet* p_tunnel = p->copy();
			bu->eface()->send(p_tunnel,0);
			debug("At %f UdpmysipAgent MR in %s send 200ok packet to %s\n", 
					NOW, MYNUM, Address::instance().print_nodeaddr(iph->daddr()));
			ih->prio_ = 14;
		}
		
		
	} else if(node_type_ == SIP_MR_HA) {
		if(mh->method==5)
		{
			if(iph->daddr()==addr()) {
				registration(p,SIP_MR);
				dump();
				debug("At %f UdpmysipAgent MR_HA in %s recv register packet\n", NOW, MYNUM);
//				if (nh->A()==ON) 
//				{
//					send_bu_ack(p);
//				}

			} else {
//				iph->saddr()=addr();
//				Packet* p_untunnel = p->copy();
//				send(p_untunnel,0);
				debug("At %f UdpmysipAgent MR_HA in %s send register packet to MN_HA or CN %s\n", 
						NOW, MYNUM, Address::instance().print_nodeaddr(iph->daddr()));
			}
		}
		if(mh->method==0)
		{
			debug("At %f UdpmysipAgent MR_HA in %s recv invite packet\n", NOW, MYNUM);
			send_temp_move_pkt(p);
			ih->prio_ = 14;
		}
	} else if(node_type_ == SIP_MN_HA) {
		if(mh->method==5)
		{
			registration(p,SIP_MN);
			dump();
			debug("At %f UdpmysipAgent MN_HA in %s recv register packet\n", NOW, MYNUM);
		}
		if(mh->method==0)
		{
			debug("At %f UdpmysipAgent MN_HA in %s recv invite packet\n", NOW, MYNUM);
			send_temp_move_pkt(p);
			ih->prio_ = 14;
		}
	} else if(node_type_ == SIP_MN) {
		if(mh->method==0)
		{
			debug("At %f UdpmysipAgent MN in %s recv invite packet\n", NOW, MYNUM);
		}
	} else if(node_type_ == SIP_CN){
		if(mh->method==6)
		{
			debug("SIP_CN temp_move\n");
			//registration(p, SipNodeType type)
			debug("At %f UdpmysipAgent CN in %s recv invite packet\n", NOW, MYNUM);
			send_invite_to_temp_move_pkt(p);
			ih->prio_ = 14;
		}
		if(mh->method==1)
		{
			hdrc->size()-=20;
		}
	}
	//----------------sem end------------------//

	// if it is a MM packet (data or ack)
	if(ih->prio_ == 15) { 
	        session_run=1;
		if(app_) 
		{  // if MM Application exists
			// re-assemble MM Application packet if segmented
//			hdr_mysip* mh = hdr_mysip::access(p);
			
			printf("node_type_ %d\n",node_type_);
			
			printf("%s Udpmysip::recv app_ ==true,asm_seq=%d,seq=%d,nbytes=%d\n",PRINTADDR(addr()),asm_info.seq,mh->seq,mh->nbytes);
			if(mh->ack && (mh->method == 0 || mh->method == 1))
			{
			    cip_ = mh->cip;
			    cport_ = mh->cport;
			    printf("udpmysip::recv cip_:%d cport_:%d\n",cip_,cport_);
			}

			if(mh->seq == asm_info.seq)
				asm_info.rbytes += hdr_cmn::access(p)->size();
			else if(mh->ack){
				asm_info.tbytes = mh->nbytes;
				asm_info.rbytes = hdr_cmn::access(p)->size();
			}					
			else {
				asm_info.seq = mh->seq;
				asm_info.tbytes = mh->nbytes;
				asm_info.rbytes = hdr_cmn::access(p)->size();
			}
			// if fully reassembled, pass the packet to application
			printf("asm_info.tbytes %d == asm_info.rbytes %d\n",asm_info.tbytes,asm_info.rbytes);
			if(asm_info.tbytes == asm_info.rbytes) {
				printf("test1");
				hdr_mysip mh_buf;
				memcpy(&mh_buf, mh, sizeof(hdr_mysip));
				printf("UdpmysipAgent\n\tsaddr() %s:%d daddr()=%s:%d\n\tnew_addr=%s\n",PRINTADDR(ih->saddr()),ih->sport(),PRINTADDR(ih->daddr()),ih->dport(),PRINTADDR(new_addr));
				app_->recv_msg(mh_buf.nbytes, (char*) &mh_buf);
			}
		}
		Packet::free(p);
	}
	// if it is a normal data packet (not MM data or ack packet)
	else { 
		printf("%s not SIP....  UdpmysipAgent recv call app_->recv(...)\n",PRINTADDR(addr()));
		if (app_) app_->recv(bytes_to_deliver);
		Packet::free(p);
	}
}

void UdpmysipAgent::info_new_addr(int newaddr)
{
   printf("%lf UdpmysipAgent::info_new_addr new_addr=%d session_run=%d\n",NOW,newaddr,session_run);
   new_addr=newaddr;
   if(session_run==1) app_->send_invite_pkt();

}

void UdpmysipAgent::send_reg_msg(int prefix, Node *iface)
{
//	SIPEntry *sip =  siplist_head_.lh_first;
//	assert(sip!=NULL);
	SIPEntry *sip = get_entry_by_iface(iface);
	assert(sip!=NULL);
	
	
	sip->con() = iface->address();
	
	Packet *p = allocpkt();
	hdr_ip *iph= HDR_IP(p);
	hdr_cmn *hdrc= HDR_CMN(p);
	hdr_mysip *mh= HDR_MYSIP(p);
	
	mh->ack = 1;  // this pregisteret is register pregisteret
	mh->time = NOW;
	mh->seq = -2;         // MM sequece number
	mh->nbytes = 200;  // register pregisteret size is 40 Bytes
	//mh->scale = p_accnt.last_scale;
	mh->method = 5;
	mh->requestURL = sip->add();
	mh->requestURL_id = sip->add_id();
	mh->From_id = sip->url_id();
	mh->From = sip->url();
	mh->To_id = sip->url_id();
	mh->To = sip->url();
	//mh->CSeq = 0;
	mh->contact_id = sip->url_id();
	mh->contact = iface->address();
	mh->cport = 3;
	
	show_sipheader(p);
	
	iph->saddr() = iface->address();
	iph->daddr() = sip->add();
	iph->dport() = port();
	
	hdrc->size() = 200;
	
	if(node_type_==SIP_MR)
	{
		debug("MR node reg \n");
		
		
		Tcl& tcl = Tcl::instance();
		tcl.evalf("%s entry", iface->name());
		NsObject* obj = (NsObject*) TclObject::lookup(tcl.result());
		Scheduler::instance().schedule(obj,p->copy(),0.1);
	
	} else {
	
		if(sip!=NULL) {
				debug("MN node reg \n");
				
				
				
			} else {
				
				debug("LFN node reg \n");
				
				
				
			}

			iph->daddr() = prefix;
			
			sip->eface()->send(p,0);
		
	}
	
	debug(
			"At %f UdpmysipAgent in %s send registraion message using interface %s\n", 
			NOW, MYNUM, Address::instance().print_nodeaddr(iface->address()));
	
}

void UdpmysipAgent::dump() {
	
	SIPEntry *node =  siplist_head_.lh_first;
	
	if (node) {
		
		cout <<"\n|"<< "SIP Binding Update List" << " for node "<< PRINTADDR(addr()) <<" at "<< NOW <<"\n";
		cout <<"|Type\tAddr \t URL_ID@URL \t CON_ID@CON \t NEMO_prefixe \t eFace \t iFace \n";

		for (; node; node=node->next_entry() ) {
			if(node->eface()!=NULL && node->iface()!=NULL) {
				
				cout <<" | "<< node->type() <<" \t \t " 
					<< node->add_id() <<"@" << PRINTADDR(node->add()) << "\t"
					<< node->url_id() <<"@" << PRINTADDR(node->url()) << "\t"
					<< node->con_id() <<"@" << PRINTADDR(node->con()) << "\t"
					<< PRINTADDR(node->prefix()) << "\t"
					<< PRINTADDR(node->eface()->get_iface()->address()) << "\t" 
					<< PRINTADDR(node->iface()->get_iface()->address()) << "\n";
				
			} else {
				
				cout <<" | "<< node->type() <<" \t \t " 
					<< node->add_id() <<"@" << PRINTADDR(node->add()) << "\t"
					<< node->url_id() <<"@" << PRINTADDR(node->url()) << "\t"
					<< node->con_id() <<"@" << PRINTADDR(node->con()) << "\t"
					<< PRINTADDR(node->prefix()) << "\n";
				
			}
		}

		cout << "\n";
	}
}

void UdpmysipAgent::show_sipheader(Packet* p)
{
	hdr_mysip *msg= HDR_MYSIP(p);
	cout	<<"sipH method: "<< msg->method << "\t"
				<<" reqURL: " << msg->requestURL_id <<"@"<< PRINTADDR(msg->requestURL) << endl
				<<" From: " << msg->From_id <<"@"<< PRINTADDR(msg->From) << endl
				<<" To: " << msg->To_id <<"@"<< PRINTADDR(msg->To)  << endl
				<<" contact: "<< msg->contact_id <<"@"<< PRINTADDR(msg->contact) << endl  
				<<" CSeq: "<< msg->CSeq << endl  
				<<" cip: "<< PRINTADDR(msg->cip) << endl
				<<" cport: "<< msg->cport << endl
				<< endl;
}

void UdpmysipAgent::registration(Packet* p, SipNodeType type)
{
//	hdr_ip *iph= HDR_IP(p);
	hdr_mysip* mh= HDR_MYSIP(p);


//	printf("UdpmysipAgent::registration hdr_mysip---> coa %s haddr %s r %d\n",
//			Address::instance().print_nodeaddr(nh->coa()), Address::instance().print_nodeaddr(nh->haddr()), nh->R() );

	if(type==SIP_MR)
	{
		SIPEntry* bu = get_entry_by_url_id(mh->contact_id);
		if(!bu)
		{
			bu = new SIPEntry(SIP_MR);
			bu->update_entry(mh->From_id,mh->From,mh->contact_id,mh->contact);
			bu->insert_entry(&siplist_head_);
		}
		bu->con()=mh->contact;

	} 
	else if (type==SIP_MN)
	{
		SIPEntry* bu = get_entry_by_url_id(mh->contact_id);
		if(!bu)
		{
			bu = new SIPEntry(SIP_MN);
			bu->update_entry(mh->From_id,mh->From,mh->contact_id,mh->contact);
			bu->insert_entry(&siplist_head_);
		}
		bu->con()=mh->contact;
	}
//	else if (type==CN)
//	{
//
//	}
}

SIPEntry* UdpmysipAgent::get_entry_by_url_id(int url_id)
{
	SIPEntry *bu =  siplist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->url_id()==url_id) {
			return bu;
		}
	}
	return NULL;
}


SIPEntry* UdpmysipAgent::get_entry_by_prefix(int prefix)
{
	SIPEntry *bu =  siplist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->prefix()==prefix) {
			return bu;
		}
	}
	return NULL;
}

SIPEntry* UdpmysipAgent::get_entry_by_iface(Node *iface)
{
	SIPEntry *bu =  siplist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->eface()->get_iface()==iface) {
			return bu;
		}
	}
	return NULL;
}

SIPEntry* UdpmysipAgent::get_entry_by_type(SipNodeType type)
{
	SIPEntry *bu =  siplist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->type()==type) {
			return bu;
		}
	}
	return NULL;
}

NEMOAgent* UdpmysipAgent::get_iface_agent_by_prefix(int prefix)
{
	SIPEntry *bu =  siplist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->prefix()==prefix) {
			return bu->iface();
		}
	}
	return NULL;
}

void UdpmysipAgent::send_temp_move_pkt(Packet* p)
{
	hdr_ip *iph= HDR_IP(p);
	hdr_mysip *mh= HDR_MYSIP(p);
	
	SIPEntry* bu = get_entry_by_url_id(mh->requestURL_id);
	if(!bu)
	{
		//	return error;
	} else {
		mh->method = 6;
		
		mh->contact_id = bu->con_id();
		mh->contact = bu->con();		
		iph->daddr() = iph->saddr();
		iph->dport() = port();
		
	}
	show_sipheader(p);
	Packet* p_temp_move = p->copy();
	send(p_temp_move,0);
	debug("At %f UdpmysipAgent MR_HA or MN_HA in %s send temp_move packet\n", NOW, MYNUM);
}

void UdpmysipAgent::send_invite_to_temp_move_pkt(Packet* p)
{
	hdr_ip *iph= HDR_IP(p);
	hdr_mysip *mh= HDR_MYSIP(p);
	
	iph->daddr()=mh->contact;
	iph->dport()=port();
	mh->method = 0;
	
	mh->requestURL_id = mh->contact_id;
	mh->requestURL = mh->contact;
//	mh->To_id = mh->contact_id;
//	mh->To = mh->contact;
	mh->contact_id = mh->From_id;
	mh->contact = mh->From;
	
	show_sipheader(p);
	Packet* p_invite = p->copy();
	send(p_invite,0);
	debug("At %f UdpmysipAgent CN in %s send invite packet\n", NOW, MYNUM);
}

