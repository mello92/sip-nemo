#include "nemo.h"

static class NEMOAgentClass : public TclClass
{
public:
	NEMOAgentClass() : TclClass("Agent/NEMO") {}
	TclObject* create(int, const char*const*) {
		return (new NEMOAgent());
	}
}class_nemoagent;


NEMOAgent::NEMOAgent() : Agent(PT_UDP)
{
	
}

int NEMOAgent::command(int argc, const char*const* argv)
{
	return (Agent::command(argc, argv));
}

void NEMOAgent::recv(Packet* p, Handler *h)
{

}

