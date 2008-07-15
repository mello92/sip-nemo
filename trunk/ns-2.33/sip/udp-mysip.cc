#include "udp-mysip.h"
#include "rtp.h"

int hdr_mysip::offset_;

//	Multimedia Header Class
static class MultimediaHeaderClass : public PacketHeaderClass {
public:
	MultimediaHeaderClass() : PacketHeaderClass("PacketHeader/Mysip", sizeof(hdr_mysip)) {
		bind_offset(&hdr_mysip::offset_);
	}
} class_mysiphdr;

//	UdpMmAgent OTcl linkage class
static class UdpmysipAgentClass : public TclClass {
public:
	UdpmysipAgentClass() : TclClass("Agent/UDP/UDPmysip") {}
	TclObject* create(int argc, const char* const* argv) {
		return(new UdpmysipAgent());
	}
} class_udpmysip_agent;

UdpmysipAgent::UdpmysipAgent():UdpAgent()
{
	support_mm_ = 0;
	asm_info.seq = -1;
}

UdpmysipAgent::UdpmysipAgent(packet_t type):UdpAgent(type)
{
	support_mm_ = 0;
	asm_info.seq = -1;
}

void UdpmysipAgent::sendmsg(int nbytes, const char* flags)
{
	printf("UdpmysipAgent send!\nsize_ %d nbytes %d\n", size_, nbytes);
	Packet *p;
	int n, remain;
	
	if(size_) {
		n = (nbytes/size_ + (nbytes%size_ ? 1 : 0 ));
		remain = nbytes%size_;		
	}
	else
		printf("Error: UDPmm size = 0\n");
	
	printf("n %d  remain %d \n",n,remain);
	
	if (nbytes==-1) {
		printf("Error: sendmsg() for UDPmm should not be -1 \n");
		return;
	}
	double local_time = Scheduler::instance().clock();
	while(n-->0) {
		p = allocpkt();
		if ( n==0 && remain>0 ) hdr_cmn::access(p)->size() = remain;
		else hdr_cmn::access(p)->size() = size_;
		hdr_rtp* rh = hdr_rtp::access(p);
		rh->flags() = 0;
		rh->seqno() = ++ seqno_;
		hdr_cmn::access(p)->timestamp() = (u_int32_t)(SAMPLERATE*local_time);
		hdr_mysip* mh = hdr_mysip::access(p);
		mh->ack = 0;
		mh->seq = 0;
		mh->nbytes = 0;
		mh->time = 0;
		mh->scale = 0;
		mh->requestURL = 0;
		mh->contact = 0;
		printf("n %d\n",n);
		if(support_mm_) {
			hdr_ip* ih = hdr_ip::access(p);
			ih->prio_ = 15;
			if(flags)	// MM Seq Num is passed as flags
			{
				printf("MM flags\n");
				memcpy(mh, flags, sizeof(hdr_mysip));
			}
			//--------------SIP add start--------------
//			ih->daddr() = Address::instance().get_nodeaddr(mh->requestURL);
//			ih->saddr() = Address::instance().get_nodeaddr(mh->contact);
//			if (!mh->ack)
//			{
//				ih->dport() = Address::instance().get_nodeaddr(cport_);
//				printf("cport_ %d\n",cport_);
//			}
//			else
//			{
//				ih->dport() = Address::instance().get_nodeaddr(mh->cport);
//				printf("mh->cport %d\n",mh->cport);
//			}
			//--------------SIP add end--------------
		}
		// add "beginning of talkspurt" labels
		if(flags && (0==strcmp(flags,"NEW_BURST")))
		{
			printf("flags2\n");
			rh->flags() |= RTP_M;
		}
		
		target_->recv(p);
		
	}
	idle();
	
}

void UdpmysipAgent::recv(Packet* p, Handler *)
{
	printf("UdpmysipAgent recive!\n");
	hdr_ip *ih = hdr_ip::access(p);
	int bytes_to_deliver = hdr_cmn::access(p)->size();
		
	if(ih->prio_ == 15) {
		if(app_) {
			//	re-assemble MM Application packet if segmented
			hdr_mysip* mh = hdr_mysip::access(p);
			
			//--------------SIP add start--------------
			if(mh->ack && (mh->method==0 || mh->method==1 ))
			{
				cip_ = mh->cip;
				cport_ = mh->cport;
				printf("cip_ %d cport_ %d\n",cip_,cport_);
			}
			//--------------SIP add end--------------
			
			if(mh->seq==asm_info.seq)
			{
				asm_info.rbytes+=hdr_cmn::access(p)->size();
				printf("asm_info.rbytes %d",asm_info.rbytes);
			}
			else{
				asm_info.seq = mh->seq;
				asm_info.tbytes = mh->nbytes;
				asm_info.rbytes = hdr_cmn::access(p)->size();
				printf("asm_info.seq %d asm_info.tbytes %d asm_info.rbytes %d \n", asm_info.seq, asm_info.tbytes, asm_info.rbytes);
			}
			
			//	if fully reassembled, pass the packet ot application
			if(asm_info.tbytes==asm_info.rbytes) {
				printf("send to Applicaiotn\n");
				hdr_mysip mh_buf;
				memcpy(&mh_buf,mh,sizeof(hdr_mysip));
				app_->recv_msg(mh_buf.nbytes, (char*) &mh_buf);
						
			}
		}
		Packet::free(p);
	}
	else {
		printf("%s not SIP \n",PRINTADDR(addr()));
		if (app_) app_->recv(bytes_to_deliver);
		Packet::free(p);
	}
}

