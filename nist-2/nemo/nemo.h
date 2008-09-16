#ifndef NEMO_H_
#define NEMO_H_

#include "agent.h"
#include "tclcl.h"
#include "packet.h"
#include "address.h"
#include "ip.h"
#include "node.h"
#include "random.h"

class NEMOAgent : public Agent {
public:
	NEMOAgent();
	int command(int argc, const char*const* argv);
	void recv(Packet*, Handler*);
	void send_bu_ack(Packet* p);
	Node* get_iface() { return iface_;}
	void send_bu_ha(Packet* p);
	
protected:
	int default_port_;
	
private:
	Node *iface_;
	
};

#endif /*NEMO_H_*/
