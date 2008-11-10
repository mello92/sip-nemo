//
// Author:    Jae Chung
// File:      mm-app.h
// Written:   07/17/99 (for ns-2.1b4a)
// Modifed:   10/14/01 (for ns-2.1b8a)
// 

//#include "timer-handler.h"
#include "../tools/random.h"
#include "packet.h"
#include "app.h"
#include "udp-mysip.h"
#include "../routing/address.h"


#define PRINTADDR(a) Address::instance().print_nodeaddr(a)

//0401
#define MAX_URL_length 20


// This is used for receiver's received packet accounting
struct pkt_accounting { 
        int last_seq;   // sequence number of last received MM pkt
        int last_scale; // rate (0-4) of last acked
        int lost_pkts;  // number of lost pkts since last ack
        int recv_pkts;  // number of received pkts since last ack
        double rtt;     // round trip time
	double last_arrival_time;
	int total_lost_pkts;

};


// URL vs IP table 0401 
class URLIPtable;
LIST_HEAD(url_ip_table,URLIPtable);

class URLIPtable
{
public:
	int url_id;
	int url;
	int IP;  
	int Port;
 	
	URLIPtable(int xurl_id, int xurl, int xIP, int xPort)
	{
	    url_id = xurl_id;
	    url = xurl;
	    IP = xIP;
	    Port = xPort;
	}
	~URLIPtable() {}
	
	URLIPtable* next_URLIPtable(void) const { return link.le_next; }

	inline void remove_URLIPtable() { LIST_REMOVE(this, link);}

	inline void insert_URLIPtable(struct url_ip_table* head)
	{
		LIST_INSERT_HEAD(head, this, link);
	}

protected:
	LIST_ENTRY(URLIPtable) link;
};




class mysipApp;
// Sender uses this timer to 
// schedule next app data packet transmission time
class SendTimer : public TimerHandler
{
 public:
	SendTimer(mysipApp* t) : TimerHandler() { t_=t; }
 protected:
	void expire(Event* e);
	mysipApp* t_;
};


// Reciver uses this timer to schedule
// next ack packet transmission time
class AckTimer : public TimerHandler
{
 public:
	AckTimer(mysipApp* t) : TimerHandler() { t_=t; }
 protected:
	void expire(Event* e);
	mysipApp* t_;
};

class OKTimer : public TimerHandler
{
 public:
	OKTimer(mysipApp* t) : TimerHandler() { t_=t; }
 protected:
	void expire(Event* e);
	mysipApp* t_;
};

class InviteTimer : public TimerHandler 
{
  public:
     InviteTimer(mysipApp* a) : TimerHandler() { a_ = a; }
  protected:
     void expire(Event* e);
     mysipApp* a_;
};

// Mulitmedia Application Class Definition
class mysipApp : public Application {
 public:
	mysipApp();
	void send_mysip_data();  // called by SendTimer:expire (Sender)
        
	//zhengjr@locust 12/23
	void send_200ok_pkt(); // called by AckTimer:expire (Receiver)
        void send_ack_for_200ok_pkt();
        void send_invite_pkt();

	// 0401 start
	struct url_ip_table url_head;
	// 0401 end
	

 protected:
	int command(int argc, const char*const* argv);
	void start();       // Start sending data packets (Sender)
	void stop();        // Stop sending data packets (Sender)


	// 0401 start
	void add_urllist(int url_id, int url, int addr, int port);
	URLIPtable* lookup_table(URLIPtable* node, int index_url_id, int index_url);
	void dump_table();
	void dump_urliptable(URLIPtable*,char*);

	URLIPtable* headu(struct url_ip_table urlhead) { return urlhead.lh_first; }
	// 0401 end

 private:
	void init();
	inline double next_snd_time();                          // (Sender)
	virtual void recv_msg(int nbytes, const char *msg = 0); // (Sender/Receiver)
	void set_scale(const hdr_mysip *mh_buf);                   // (Sender)
	void set_contact(const hdr_mysip *mh_buf);
	void adjust_scale(void);                                // (Receiver)
	void account_recv_pkt(const hdr_mysip *mh_buf);            // (Receiver)
	void init_recv_pkt_accounting();                        // (Receiver)

	double rate[5];        // Transmission rates associated to scale values
	double interval_;      // Application data packet transmission interval
	int pktsize_;          // Application data packet size
	int random_;           // If 1 add randomness to the interval
	int running_;          // If 1 application is running
	int seq_;              // Application data packet sequence number
	int scale_;            // Media scale parameter
	pkt_accounting p_accnt;
	SendTimer snd_timer_;  // SendTimer
	AckTimer  ack_timer_;  // AckTimer
	OKTimer  ok_timer_;  // OKTimer
	InviteTimer invite_timer_;
	int method_;
        int requestURL_;
                                                                                
        int From_;
        int To_;
        int CSeq_;
        int contact_;
	int contact_dst_addr;
	void show_sipheader(struct hdr_mysip*);
	int invite_stop;
	int m200ok_stop;
	int ack_stop;
	//0403 start
	int myID;
	int myURL;
	int toID;
	int toURL;
	void myID_URL();
	void send_register_pkt();
	void send_temp_move_pkt();
	struct hdr_mysip* last_siph;
	char filename[30];
	char filename_sec[30];
	char filename_all[30];
	double handofftime[30];
	double error_recover_time[30];
	int handoffnum;
	int error_recover_num;
	double last_reinvite_time;
	void dump_handoff();
	void measure_handoff();
	
	char* new_addr;

	//0403 end
	
	double first_seq_time;

};



