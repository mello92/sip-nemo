/*
 * include file for Interface management module
 *
 * This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to title 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * NIST assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.
 * <BR>
 * We would appreciate acknowledgement if the software is used.
 * <BR>
 * NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 * </PRE></P>
 * @author  rouil
 * @version 1.0
 * <P><PRE>
 * VERSION CONTROL<BR>
 * -------------------------------------------------------------------------<BR>
 * Name  - YYYY/MM/DD - VERSION - ACTION<BR>
 * rouil - 2005/05/01 - 1.0     - Source created<BR>
 * <BR>
 * <BR>
 * </PRE><P>
 */ 

#ifndef mip6_h
#define mip6_h

#include "if-mngmt.h"
#include <vector>

//----------------sem start------------------//
#include "udp-mysip.h"
#include "nemo.h"
//----------------sem end------------------//
	
#define MYNUM Address::instance().print_nodeaddr(addr())
#define HADDR Address::instance().get_nodeaddr(addr())
#define PRINTADDR(a) Address::instance().print_nodeaddr(a)

#define NONE				-1
#define ON					1
#define OFF					0

//defines the type of timers. Timer ID >= 0 are reserved for prefix
#define MIP_TIMER_BU		-3
#define MIP_TIMER_BSL		-4
#define MIP_TIMER_RA		-5

#define RA_MAX_AGE			7
#define MAX_UPDATE_RATE		1
#define SLOW_UPDATE_RATE	10
#define MAX_FAST_UPDATES	5
#define LIFETIME			reglftm_
#define TIME_INFINITY		0x0fffffff

#define BU_OPT_SIZE			10
#define BU_ACK_OPT_SIZE		13
#define BU_REQ_OPT_SIZE		2
#define HOME_OPT_SIZE		18
#define DEST_HDR_PREFIX_SIZE 2

#define BU_SIZE			DEST_HDR_PREFIX_SIZE + BU_OPT_SIZE + HOME_OPT_SIZE
#define BACK_SIZE		DEST_HDR_PREFIX_SIZE + BU_ACK_OPT_SIZE
#define BREQ_SIZE		DEST_HDR_PREFIX_SIZE + BU_REQ_OPT_SIZE

typedef enum {
	MN, MN_HA, MR, MR_HA, CN, MR_BS
} Mipv6NodeType;

typedef enum {
	BREQ, BU, BACK, REHOME, BU_HA, BU_BS, BU_CN, BU_RP, BU_MN,
	BS_ADS, BS_SOL, BU_MR, REHOME_MR_HA, BU_MR_HA
} Mipv6RegType;

/****************************************
 definition of BU list
 2003/9/26 hscho@mmlab.snu.ac.kr
 class Entry;
****************************************/

class MEMOAgent;

class BUEntry;

LIST_HEAD(bu_entry, BUEntry);

class BUEntry
{
public:
	Mipv6NodeType type_;
	int addr_;
	int haddr_;
	int caddr_;
	int bid_;
	int nemo_prefix_;
	NEMOAgent *eface_agent_;
	NEMOAgent *iface_agent_;

	BUEntry(Mipv6NodeType type)
	{
		type_ = type;
		addr_ = -1;
		haddr_ = -1;
		caddr_ = -1;
		bid_ = -1;
		nemo_prefix_ = -1;
		eface_agent_ = NULL;
		iface_agent_ = NULL;
	}
	
	~BUEntry() {}

	inline void insert_entry(struct bu_entry* head)
	{
		LIST_INSERT_HEAD(head, this, link);
	}
	
	inline void update_entry(int addr, int haddr)
	{
		addr_ = addr;
		haddr_ = haddr;
	}
	inline void update_entry(int haddr, int caddr, int nemo_prefix)
	{
		haddr_ = haddr;
		caddr_ = caddr;
		nemo_prefix_ = nemo_prefix;
	}
	inline void update_entry(int addr, int haddr, NEMOAgent *eface_agent)
	{
		addr_ = addr;
		haddr_ = haddr;
		eface_agent_ = eface_agent;
	}
	inline void update_entry(int addr, int haddr, int nemo_prefix, NEMOAgent *eface_agent, NEMOAgent *iface_agent)
	{
		addr_ = addr;
		haddr_ = haddr;
		nemo_prefix_ = nemo_prefix;
		eface_agent_ = eface_agent;
		iface_agent_ = iface_agent;
	}
	inline void update_entry(int addr, int haddr, int caddr, int bid, int nemo_prefix, NEMOAgent *eface_agent, NEMOAgent *iface_agent)
	{
		addr_ = addr;
		haddr_ = haddr;
		caddr_ = caddr;
		bid_ = bid;
		nemo_prefix_ = nemo_prefix;
		eface_agent_ = eface_agent;
		iface_agent_ = iface_agent;
	}
	inline void set_mface(int nemo_prefix, NEMOAgent *iface_agent)
	{
		nemo_prefix_ = nemo_prefix;
		iface_agent_ = iface_agent;
	}
	inline void set_mface_2(int neighor_mr, int nemo_prefix, NEMOAgent *iface_agent)
	{
		haddr_ = neighor_mr;
		nemo_prefix_ = nemo_prefix;
		iface_agent_ = iface_agent;
	}
	
	inline void set_mr_bs(NEMOAgent *eface_agent, NEMOAgent *iface_agent)
	{
		eface_agent_ = eface_agent;
		iface_agent_ = iface_agent;
	}
	
	inline Mipv6NodeType& type() { return type_; }
	inline int& addr() { return addr_;}
	inline int& haddr() { return haddr_;}
	inline int& caddr() { return caddr_; }
	inline int& prefix() { return nemo_prefix_;} 
	inline NEMOAgent* eface() { return eface_agent_;}
	inline NEMOAgent* iface() { return iface_agent_;}

//	inline double expire() { return (time + lftm); }
//	inline double& lifetime() { return lftm; }
//	inline int activated(double now) {
//		return ((flag == ON) && (now<active_expire))?TRUE:FALSE;
//	}
//	inline void activate_entry(double now, double life) {
//		flag = ON;
//		if ( (now+life)>active_expire )
//		{
//			active_expire = now + life;
//		}
//	}

	BUEntry* next_entry(void) const { return link.le_next; }
	inline void remove_entry() { LIST_REMOVE(this, link);}
//	inline void remove_entry(struct bu_entry* newhead)
//	{
//		LIST_REMOVE(this, link);
//		LIST_INSERT_HEAD(newhead, this, link);
//	}
//	inline void deactivate_entry() { flag = OFF; }

protected:
	LIST_ENTRY(BUEntry) link;
};

//	---------------- sem start ----------------
/*
 * BS list
 * Store information about a neighbor: prefix, lifetime. 
 */
class BSEntry;

LIST_HEAD(bs_entry, BSEntry);

class BSEntry {
public:
	
	  int prefix_;            // router prefix
	  double time_; 	  // lifetime of this entry 
	  
	  BSEntry(int prefix, double lifetime)  { //constructor
	    prefix_ = prefix;	
	    time_ = lifetime;
	  }
	  ~BSEntry() { ; } //destructor
	  
	  // Chain element to the list
	  inline void insert_entry(struct bs_entry *head) {
	    LIST_INSERT_HEAD(head, this, link);
	  }
	  inline void update_entry(double lifetime) {
	    time_ = lifetime;
	  }
	  inline double& lifetime() { return time_; }
	  // Return the prefix information
	  inline int& prefix() {return prefix_; }
	  // Return next element in the chained list
	  BSEntry* next_entry(void) const { return link.le_next; }
	  // Remove the entry from the list
	  inline void remove_entry() { 
		  LIST_REMOVE(this, link); 
	  }
	  
protected:
	LIST_ENTRY(BSEntry) link; 
};
//	---------------- sem end ----------------

// TunnelEntry

struct tunnel {
	nsaddr_t saddr;
	int32_t sport;
	nsaddr_t daddr;
	int32_t dport;
};

#define HDR_TUNNEL(p) ((struct hdr_tunnel*)(p)->access(hdr_tunnel::offset_))
struct hdr_tunnel
{
	vector <struct tunnel*> tunnels_;
	int num_;
	static int offset_;
	inline static int& offset() { return offset_; }
	inline static hdr_tunnel* access(Packet* p) {
		return (hdr_tunnel*) p->access(offset_);
	}
	inline int& num() { return num_; }
	inline vector<struct tunnel*>& tunnels() { return tunnels_;}
};

#define PT_TUNNEL_SIZE		IPv6_HEADER_SIZE

/****************************************
 definition of NEMO Message
 2003/9/26 hscho@mmlab.snu.ac.kr
****************************************/
#define HDR_NEMO(p) ((struct hdr_nemo*)(p)->access(hdr_nemo::offset_))
struct hdr_nemo
{
	Mipv6RegType type_;
	int A_;
	int H_;
	int R_;
	int seq_;
	double lftm_;
	int nemo_prefix_;
	int homeaddr_;
	int coaddr_;

	static int offset_;
	inline static int& offset() { return offset_; }
	inline static hdr_nemo* access(Packet* p) {
		return (hdr_nemo*) p->access(offset_);
	}

	inline Mipv6RegType& type() { return type_; }
	inline int& A() { return A_; }
	inline int& H() { return H_; }
	inline int& R() { return R_; }
	inline int& seqno() { return seq_; }
	inline double& lifetime() { return lftm_; }
	inline int& coa() { return coaddr_; }
	inline int& haddr() { return homeaddr_; }
	inline int& nemo_prefix() { return nemo_prefix_; }
};

/* 
 * Packet structure for redirect messages
 */
#define HDR_RTRED(p)    ((struct hdr_rtred*)(p)->access(hdr_rtred::offset_))

struct hdr_rtred {

  vector <Agent*> agent_;  //agents to redirect
  int dest_;      //new interface to use
  char ack_;      //define if it is an ack message

  static int offset_;
  inline static int& offset() { return offset_; }
  inline static hdr_rtred* access(Packet* p) {
    return (hdr_rtred*) p->access(offset_);
  }
  inline vector<Agent*>& agent() { return agent_; }
  inline int& destination() { return dest_; }
  inline char& ack() { return ack_; }
};
#define PT_RRED_SIZE		IPv6_HEADER_SIZE + 8

/* 
 * Packet structure for flow request
 */
#define HDR_FREQ(p)    ((struct hdr_freq*)(p)->access(hdr_freq::offset_))

struct hdr_freq {
  vector <FlowEntry*> flow_info;
  int seqNumber_;

  static int offset_;
  inline static int& offset() { return offset_; }
  inline static hdr_freq* access(Packet* p) {
    return (hdr_freq*) p->access(offset_);
  }
};
#define PT_FREQ_SIZE		IPv6_HEADER_SIZE + 8

/* 
 * Packet structure for flow request
 */
#define HDR_FRES(p)    ((struct hdr_fres*)(p)->access(hdr_fres::offset_))

struct hdr_fres {
  vector <FlowEntry*> flow_info;
  int seqNumber_;

  static int offset_;
  inline static int& offset() { return offset_; }
  inline static hdr_fres* access(Packet* p) {
    return (hdr_fres*) p->access(offset_);
  }
};
#define PT_FRES_SIZE		IPv6_HEADER_SIZE + 8


/*
 * Packet structure for communicating between Handover modules
 */
#define HDR_HAND(p) ((struct hdr_hand*)(p)->access(hdr_hand::offset_))

struct prefered_network {
  int type;
  int mac;
  int poa;
  int channel;
};

struct hdr_hand {
  int subtype;  //for different content

  //the following are used for communication handover preferences
  //between PA and MN
  vector <prefered_network*> *networks;

  static int offset_;
  inline static int& offset() { return offset_; }
  inline static hdr_hand* access(Packet* p) {
    return (hdr_hand*) p->access(offset_);
  }
};
//define size containing only 1 entry in the vector. If more, redefine
#define PT_HAND_SIZE		IPv6_HEADER_SIZE + 40

/* Data structure to keep track of who is connected
 */
struct list_node {
  Node *node; //the node connected
  int port; //the port of the application
  float minBw;    //minimum bandwidth used by the application
};

/* 
 * NC flow resquest timer
 * Allow a flow request to be resent if lost
 */
class FlowRequestTimer;

class UdpmysipAgent;

class NEMOAgent;
/* 
 * MIPv6 agent
 */
class MIPV6Agent : public IFMNGMTAgent {
 public:
  MIPV6Agent();

  int command(int argc, const char*const* argv);
  void recv(Packet*, Handler*);
  //process events from the neighbor discovery module
  void process_nd_event (int, void*); 
  void process_get_status_response (mih_get_status_s*);
  void send_update_msg (Agent *, Node *);
  void send_update_msg (vector<Agent *>, Node *);
  void send_flow_request (vector<FlowEntry*>, Node *, int);
  //void process_client_going_down (int); //to remove
  void send_rs (Mac *); //send an RA message for the given interface
  
	void add_tunnel(Packet* p);
	void delete_tunnel(Packet* p);
	int size_tunnel(Packet* p);
	
	int compute_new_address (int prefix, Node *interface);
  
  //----------	muliple router use-------------//
  void process_mr(new_prefix*);
  void set_mr_bs_prefix(new_prefix* data, double lifetime);
  BUEntry* get_mr_bs_entry_by_mface(Node *mface);
//  int mr_bs_;
  int mr_bs_daddr;	//	use for mn to set bs addr
  BUEntry* get_mr_bs_entry();
  void add_mr_bs_prefix(int prefix, double lifetime);
  bool is_daddr_mr_bs_prefix_(int daddr);
  int ms_bs_enable;	//	use for mn to get only one address
  
  //-----------binding---------
  void mn_send_bu_cn();
  int mn_flag;
  
  
  //flow request timer
  FlowRequestTimer *flowRequestTimer_;
  double flowRequestTimeout_;
  int seqNumberFlowRequest_;
  
  //----------------sem start------------------//
//  void send_bu_msg(Node *iface);
//  int v_to_v ;
  void send_bu_msg(int prefix, Node *iface);
  
  void send_mr_bu_msg(int prefix, Node *iface);
  
	void tunneling(Packet* p);
	void re_homing(Node *iface);
  //----------------sem end------------------//
  
 protected:
  
  virtual void process_new_prefix (new_prefix*);
  virtual void process_exp_prefix (exp_prefix*);

  virtual void process_mr_prefix (new_prefix*);

  // Message processing
  void recv_redirect(Packet*);
  void recv_redirect_ack(Packet*);
  void recv_flow_request(Packet*);
  void recv_flow_response(Packet*);
  
//  inline int get_ha() { return ha_; }
  
  //----------------sem start------------------//
	UdpmysipAgent* udpmysip_;
//  NEMOAgent *nemo_;
	int exp_;
	int exp_mr_;
	//----------------sem end------------------//

 private:
	 //----------------sem start------------------//
	 
	 struct bu_entry bulist_head_;
	 
		void dump();
		
//		BUEntry* lookup_coa(int addr, BUEntry* n);
//		BUEntry* lookup_hoa(int addr, BUEntry* n);
//		BUEntry* lookup_entry(int addr, int coa, BUEntry* n);
//		BUEntry* head(struct bu_entry head) { return head.lh_first; }
		
		void update_bu_entry(Packet* p, int flag);
		
		void send_bu_msg();
		void send_cn_bu_msg(Packet* p, int prefix);
		
		void registration(Packet* p, Mipv6NodeType type);
		void send_bu_ack(Packet* p);
		
//		void send_bu_ack(Packet* p, Node* iface);
		void recv_bu_ack(Packet* p);
		void recv_nemo(Packet* p);
		
		  
		NEMOAgent* get_iface_agent_by_prefix(int prefix);
		NEMOAgent* get_eface_agent_by_prefix(int prefix);
		BUEntry* get_entry_by_iface(Node *iface);
//		BUEntry* get_entry_without_iface(Node *iface);
		BUEntry* get_entry_by_prefix(int prefix);
		BUEntry* get_entry_by_haddr(int haddr);
		BUEntry* get_entry_by_addr(int addr);
		BUEntry* get_entry_by_type(Mipv6NodeType type);
		BUEntry* get_mr_ha_entry_by_caddr(int caddr);
		BUEntry* get_mr_ha_entry_by_iface(Node *iface);
		BUEntry* get_mr_ha_entry_without_iface(Node *iface);
		
		BUEntry* get_mr_ha_entry_by_prefix(int prefix);
		
		BUEntry* get_mr_entry_by_prefix(int prefix);
		BUEntry* get_mr_entry();
		BUEntry* get_mr_ha_entry_on();
		BUEntry* get_mr_ha_entry_off();
		
		vector <BUEntry*> get_mr_ha_entry_dead();
		
		BUEntry* get_entry_by_caddr(int caddr);
		
		
		
//		int hoa_;	//	home address
//		int ha_;		//	home aget address
//		int coa_;	//	care-of address
//		int nemo_prefix_;	// nemo prefix
		
		Mipv6NodeType node_type_;
		
		//----------	muliple router use-------------//
		struct bs_entry bslist_head_;
		  

//		Node* iface_node_;
	 //----------------sem end------------------//
	 
  vector <list_node*> list_nodes;

  float get_used_bw();
  int nbAccepted_;
  int nbRefused_;

};
/* 
 * NC flow request timer
 * Allow a flow request to be resent if lost
 */
class FlowRequestTimer : public TimerHandler {
 public:
  FlowRequestTimer(MIPV6Agent *m) : TimerHandler() { m_ = m;}
  inline void expire(Event *){m_->send_flow_request (flows_, iface_, destination_);}
  inline void attach_event(vector<FlowEntry*> flows, Node *iface, int destination){
    flows_ = flows;
    iface_ = iface;
    destination_ = destination;
  }
  protected:
  MIPV6Agent *m_;
  vector<FlowEntry*> flows_;
  Node *iface_;
  int destination_;
};

#endif
