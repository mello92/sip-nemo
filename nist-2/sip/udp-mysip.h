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


#include "mip6.h"
#include "nemo.h"

typedef enum {
	SIP_MN, SIP_MN_HA, SIP_MR, SIP_MR_HA, SIP_CN
} SipNodeType;

class MEMOAgent;

class SIPEntry;

LIST_HEAD(sip_entry, SIPEntry);

class SIPEntry
{
public:
	SipNodeType type_;
	int addr_id_;
	int addr_;
	int url_id_;
	int url_;
	int contact_id_;
	int contact_;
	
	int cid_;
	int nemo_prefix_;
	NEMOAgent *eface_agent_;
	NEMOAgent *iface_agent_;

	SIPEntry(SipNodeType type)
	{
		type_ = type;
		addr_id_ = -1;
		addr_ = -1;
		url_id_ = -1;
		url_ = -1;
		contact_id_= -1;
		contact_ = -1;
		nemo_prefix_ = -1;
		eface_agent_ = NULL;
		iface_agent_ = NULL;
	}
	
	~SIPEntry() {}

	inline void insert_entry(struct sip_entry* head)
	{
		LIST_INSERT_HEAD(head, this, link);
	}
	
	inline void cn_init_entry(int addr_id, int addr, int url_id, int url)
	{
		addr_id_ = addr_id;
		addr_ = addr;
		url_id_ = url_id;
		url_ = url;
	}
	
	inline void mn_init_entry(int addr_id, int addr, int url_id, int url, NEMOAgent *eface_agent)
	{
		addr_id_ = addr_id;
		addr_ = addr;
		url_id_ = url_id;
		url_ = url;
		eface_agent_ = eface_agent;
	}
	
	inline void mr_init_entry(int addr_id, int addr, int url_id, int url, int nemo_prefix, NEMOAgent *eface_agent, NEMOAgent *iface_agent)
	{
		addr_id_ = addr_id;
		addr_ = addr;
		url_id_ = url_id;
		url_ = url;
		nemo_prefix_ = nemo_prefix;
		eface_agent_ = eface_agent;
		iface_agent_ = iface_agent;
	}
	
	inline void update_entry(int url_id, int url, int contact_id, int contact)
	{
		url_id_ = url_id;
		url_ = url;
		contact_id_= contact_id;
		contact_ = contact;
	}
	
	inline void update_entry(int addr_id, int addr, int url_id, int url, int contact_id, int contact, int nemo_prefix, NEMOAgent *eface_agent, NEMOAgent *iface_agent)
	{
		addr_id_ = addr_id;
		addr_ = addr;
		url_id_ = url_id;
		url_ = url;
		contact_id_= contact_id;
		contact_ = contact;
		nemo_prefix_ = nemo_prefix;
		eface_agent_ = eface_agent;
		iface_agent_ = iface_agent;
	}
	
	inline void set_mface(int nemo_prefix, NEMOAgent *iface_agent)
	{
		nemo_prefix_ = nemo_prefix;
		iface_agent_ = iface_agent;
	}
	

	inline SipNodeType& type() { return type_; }
	inline int& add_id() { return addr_id_;}
	inline int& add() { return addr_; }
	inline int& url_id() { return url_id_;}
	inline int& url() { return url_;}
	inline int& con_id() { return contact_id_; }
	inline int& con() { return contact_; }
	inline int& prefix() { return nemo_prefix_;} 
	inline NEMOAgent* eface() { return eface_agent_;}
	inline NEMOAgent* iface() { return iface_agent_;}

	SIPEntry* next_entry(void) const { return link.le_next; }
	inline void remove_entry() { LIST_REMOVE(this, link);}

protected:
	LIST_ENTRY(SIPEntry) link;
};

#define HDR_MYSIP(p) ((struct hdr_mysip*)(p)->access(hdr_mysip::offset_))

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
	
	void send_reg_msg(int prefix, Node *iface);
	void send_reg_msg();
	void re_homing(Node *iface);
	
	//	muliple router use
	void process_mr(int prefix, Node *iface);
	
protected:
	int support_mm_; // set to 1 if above is mysipApp
	
	MIPV6Agent *mipv6_;
	SipNodeType node_type_;
//	mysipApp *mysipapp_;
	
	int select_;
	
private:
	//----------------sem start------------------//
	struct sip_entry siplist_head_;
	void dump();
	void show_sipheader(Packet* p);
	void registration(Packet* p, SipNodeType type);
	SIPEntry* get_entry_by_url_id(int url_id);
	SIPEntry* get_entry_by_url(int url);
	SIPEntry* get_entry_by_prefix(int prefix);
	SIPEntry* get_entry_by_iface(Node *iface);
	SIPEntry* get_entry_without_iface(Node *iface);
	SIPEntry* get_entry_by_type(SipNodeType type);
	NEMOAgent* get_iface_agent_by_prefix(int prefix);
	
	SIPEntry* get_mr_ha_entry_by_caddr(int caddr);
	SIPEntry* get_mn_entry_by_url_id(int url_id);
	SIPEntry* get_cn_entry_by_url_id(int url_id);
	
	SIPEntry* get_mr_ha_entry_random();
	vector <SIPEntry*> rehome_mn_coa_entry_random(int caddr);
	vector <SIPEntry*> renew_mn_coa_entry_random(int caddr);
	
	SIPEntry* get_mr_ha_entry_round_robin();
	vector <SIPEntry*> rehome_mn_coa_entry_round_robin(int caddr);
	vector <SIPEntry*> renew_mn_coa_entry_round_robin(int caddr);
	int round_count;
	
	SIPEntry* get_mr_ha_entry_weight();
	vector <SIPEntry*> rehome_mn_coa_entry_weight(int caddr);
	vector <SIPEntry*> renew_mn_coa_entry_weight(int caddr);
	
	SIPEntry* get_mr_ha_entry_weight_2();
	vector <SIPEntry*> rehome_mn_coa_entry_weight_2(int caddr);
	vector <SIPEntry*> renew_mn_coa_entry_weight_2(int caddr);
	int weight_count;
	int weight_count2;
	
	void send_temp_move_pkt(Packet* p);
	void send_invite_to_temp_move_pkt(Packet* p);
	
	//	multiple router
	SIPEntry* get_mr_entry_by_prefix(int prefix);
	
	
	asm_mm asm_info; // packet re-assembly information
	int session_run;
	int flag;
};

#endif
