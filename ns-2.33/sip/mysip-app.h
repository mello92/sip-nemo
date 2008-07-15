#ifndef MYSIPAPP_H_
#define MYSIPAPP_H_

#include "app.h"
#include "udp-mysip.h"

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
	inline void remove_URLIPtable() { LIST_REMOVE(this,link); }
	inline void insert_URLIPtable(struct url_ip_table* head	)
	{
		LIST_INSERT_HEAD(head, this, link);
	}
protected:
	LIST_ENTRY(URLIPtable) link;
};

//	This is used for receiver's received packet accounting
struct pkt_accounting {
	int last_seq;
	int last_scale;
	int lost_pkts;
	int recv_pkts;
	double rtt;
};

class mysipApp;

class SendTimer : public TimerHandler {
public:
	SendTimer (mysipApp* t) : TimerHandler(), t_(t) {}
	inline virtual void expire (Event*);
protected:
	mysipApp* t_;
};

class AckTimer : public TimerHandler {
public:
	AckTimer (mysipApp* t) : TimerHandler(), t_(t) {}
	inline virtual void expire (Event*);
protected:
	mysipApp* t_;
};

class OKTimer : public TimerHandler {
public:
	OKTimer (mysipApp* t) : TimerHandler(), t_(t) {}
	inline virtual void expire (Event*);
protected:
	mysipApp* t_;
};

class InviteTimer : public TimerHandler {
public:
	InviteTimer (mysipApp* t) : TimerHandler(), t_(t) {}
	inline virtual void expire (Event*);
protected:
	mysipApp* t_;
};


class mysipApp : public Application {
public:
	mysipApp();
	void send_mm_pkt();
	void send_ack_pkt();
	
	//--------------SIP add start--------------
	void send_mysip_data();
	void send_200ok_pkt();
	void send_ack_for_200ok_pkt();
	void send_invite_pkt();
	void send_temp_move_pkt();
	
	struct url_ip_table url_head;
	//--------------SIP add end--------------
	

protected:
	int command(int argc, const char* const* argv);
	void start();
	void stop();
	
	//--------------SIP add start--------------
	void add_urllist(int url_id, int url, int addr, int port);
	URLIPtable* lookup_table(URLIPtable* node, int index_url_id, int index_url);
	void dump_table();
	void dump_urliptable(URLIPtable*, char*);
	URLIPtable* headu(struct url_ip_table urlhead) { return urlhead.lh_first;	}
	//--------------SIP add end--------------
	
private:
	void init();
	inline double next_snd_time();
	virtual void recv_msg(int nbytes, const char *msg = 0);
	
	void set_scale(const hdr_mysip* mh_buf);
	void adjust_scale(void);
	void account_recv_pkt(const hdr_mysip* mh_buf);
	void init_recv_pkt_accounting();
	
	double rate[5];
	double interval_;
	int pktsize_;
	int random_;
	int running_;
	int seq_;
	int scale_;
	
	pkt_accounting p_accnt;
	SendTimer snd_timer_;
	AckTimer ack_timer_;
	
	//--------------SIP add start--------------
	OKTimer ok_timer_;
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
	
	int myID;
	int myURL;
	int toID;
	int toURL;
	//void myID_URL();
	void send_register_pkt();
	void send_tmp_move_pkt();
	struct hdr_mysip* last_siph;
	char filename[30];
	char filename_sec[30];
	double handofftime[30];
	double error_recover_time[30];
	int handoffnum;
	int error_recover_num;
	double last_reinvite_time;
	void dump_handoff();
	void measure_handoff();
	void set_contact(const hdr_mysip* sip_buf);
	//--------------SIP add end--------------
};

#endif /*MYSIPAPP_H_*/
