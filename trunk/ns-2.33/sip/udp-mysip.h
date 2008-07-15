#ifndef UDPMYSIP_H_
#define UDPMYSIP_H_

#include "udp.h"
#include "ip.h"
#include <address.h>
#include <iostream>

#define PRINTADDR(a) Address::instance().print_nodeaddr(a)
#define STR2ADDR(a) Address::instance().str2addr(a)

//	Multimedia Header Structure
struct hdr_mysip {
	int ack;
	int seq;
	int nbytes;
	double time;
	int scale;
	
	//--------------SIP add start--------------
	int type;
	
	int method;
	int requestURL_id;
	int requestURL;
	
	int From_id;
	int From;
	int To_id;
	int To;
	int CSeq;
	int CallID;
	int contact_id;
	int contact;
	
	int cip;
	int cport;
	//--------------SIP add end--------------
	
	//	Packet header access functions
	static int offset_;
	inline static int& offset() { return offset_; }
	inline static hdr_mysip* access(const Packet* p) {
		return (hdr_mysip*) p->access(offset_);
	}
};

// Used for Re-assemble segmented Mysip packet
struct asm_mm {
	int seq;
	int rbytes;
	int tbytes;
};

//	UdpMmAgent Class
class UdpmysipAgent : public UdpAgent {
public:
	UdpmysipAgent();
	UdpmysipAgent(packet_t);
	//~UdpMmAgent();
	virtual int supportMM()	{ return 1; }
	virtual void enableMM() { support_mm_ = 1; }
	virtual void sendmsg(int nbytes, const char* flags = 0);
	void recv(Packet*, Handler*);
	
	//--------------SIP add start--------------
	int cip_;
	int cport_;
	//--------------SIP add end--------------
	
protected:
	int support_mm_;
private:
	asm_mm asm_info;
};
#endif /*UDPMYSIP_H_*/
