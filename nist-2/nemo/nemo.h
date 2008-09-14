#ifndef NEMO_H_
#define NEMO_H_

#include "agent.h"

class NEMOAgent : public Agent {
public:
	NEMOAgent();
	int command(int argc, const char*const* argv);
	void recv(Packet*, Handler*);
};

#endif /*NEMO_H_*/
