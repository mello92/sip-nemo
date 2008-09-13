//
// Author:    Jae Chung
// File:      udp-mm.h 
// Written:   07/17/99 (for ns-2.1b4a)
// Modifed:   10/14/01 (for ns-2.1b8a)
// 

#ifndef ns_udp_mm_h
#define ns_udp_mm_h

#include <iostream>
#include "udp.h"
#include "ip.h"
#include "../routing/address.h"
#define PRINTADDR(a) Address::instance().print_nodeaddr(a)
#define STR2ADDR(a) Address::instance().str2addr(a)

#include "../hsntg/mip6.h"

// Multimedia Header Structure
struct hdr_mysip {
	int type;
	int ack;     // is it ack packet?
	int seq;     // mm sequence number
	int nbytes;  // bytes for mm pkt
	double time; // current time
	int scale;   // scale (0-4) associated with data rates
	
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

	// Packet header access functions
        static int offset_;
        inline static int& offset() { return offset_; }
        inline static hdr_mysip* access(const Packet* p) {
                return (hdr_mysip*) p->access(offset_);
        }
};


// Used for Re-assemble segmented (by UDP) MM packet
struct asm_mm { 
	int seq;     // mm sequence number
	int rbytes;  // currently received bytes
	int tbytes;  // total bytes to receive for MM packet
};

class MIPV6Agent;

// UdpmysipAgent Class definition
class UdpmysipAgent : public UdpAgent {
public:
	
	int command(int argc, const char*const* argv);
	
	UdpmysipAgent();
	UdpmysipAgent(packet_t);
	virtual int supportMM() { return 1; }
	virtual void enableMM() { support_mm_ = 1; }
	virtual void sendmsg(int nbytes, const char *flags = 0);
	void recv(Packet*, Handler*);
	inline int get_new_addr(){ return new_addr; }
	void info_new_addr(int newaddr);
	int cip_;
	int cport_;
	
protected:
	int support_mm_; // set to 1 if above is mysipApp
	
	MIPV6Agent *mipv6_;
	
private:
	asm_mm asm_info; // packet re-assembly information
	int session_run;
};

#endif
