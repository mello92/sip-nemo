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
UdpmysipAgent::UdpmysipAgent() : UdpAgent(),mipv6_(NULL)
{
	bind("packetSize_",&size_);
	support_mm_ = 0;
	session_run = 0;
	//new_addr = 0;
	printf("UdpmysipA() new_addr=%d\n",new_addr);
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
		printf("UdpmysipAgent\n\tsaddr() %s:%d daddr()=%s:%d\n\tnew_addr=%s\n",PRINTADDR(ih->saddr()),ih->sport(),PRINTADDR(ih->daddr()),ih->dport(),PRINTADDR(new_addr));
		
		if(mipv6_!=0 )
		{
			printf("mipv6 enable\n");
			mipv6_->tunneling(p);
		}
		else	

		target_->recv(p);
		
	}
	idle();
}


// Support Packet Re-Assembly and Multimedia Application
void UdpmysipAgent::recv(Packet* p, Handler*)
{
	printf("recv!!\n");
	hdr_ip* ih = hdr_ip::access(p);
	int bytes_to_deliver = hdr_cmn::access(p)->size();
	//printf("UdpmysipAgent recv ih->prio_ ==%d\n",ih->prio_);

	// if it is a MM packet (data or ack)
	if(ih->prio_ == 15) { 
	        session_run=1;
		if(app_) 
		{  // if MM Application exists
			// re-assemble MM Application packet if segmented
			hdr_mysip* mh = hdr_mysip::access(p);
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
