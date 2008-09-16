#include "nemo.h"
#include "mip6.h"

#define MYNUM Address::instance().print_nodeaddr(addr())

static class NEMOAgentClass : public TclClass
{
public:
	NEMOAgentClass() : TclClass("Agent/NEMO") {}
	TclObject* create(int, const char*const*) {
		return (new NEMOAgent());
	}
}class_nemoagent;


NEMOAgent::NEMOAgent() : Agent(PT_UDP) , iface_(NULL)
{
	bind("default_port_",&default_port_);
}

int NEMOAgent::command(int argc, const char*const* argv)
{
	if (argc==3)
	{
		if(strcmp(argv[1], "connect-interface")==0) {
			iface_ = (Node *)TclObject::lookup(argv[2]);
			if(iface_==NULL)
				return TCL_ERROR;
			
			Tcl& tcl = Tcl::instance();
			tcl.evalf("%s target [%s entry]", this->name(), iface_->name());
			
			return TCL_OK;
		}
	}
	return (Agent::command(argc, argv));
}

void NEMOAgent::recv(Packet* p, Handler *h)
{
	debug ("At %f in %s NMEO Agent received packet\n", NOW, MYNUM);
}

void NEMOAgent::send_bu_ack(Packet* p)
{
	hdr_ip *iph= HDR_IP(p);
	hdr_nemo *nh= HDR_NEMO(p);
	
	//reply packet
	Packet *p_ack = allocpkt();
	hdr_ip *iph_ack= HDR_IP(p_ack);
	hdr_nemo *nh_ack= HDR_NEMO(p_ack);
	hdr_cmn *hdrc_ack= HDR_CMN(p_ack);

	nh_ack->type()=BACK;
	nh_ack->lifetime()=0;
	nh_ack->seqno()=nh->seqno();

	iph_ack->saddr() = addr();
	iph_ack->daddr() = iph->saddr();
	iph_ack->dport() = iph->sport();
	hdrc_ack->ptype() = PT_NEMO;
	hdrc_ack->size() = IPv6_HEADER_SIZE + BACK_SIZE;
	
	
	debug("iph_ack->daddr() %s aport() %d saddr() %s sport() %d \n", 
			Address::instance().print_nodeaddr(iph_ack->daddr()), iph_ack->dport(),
			Address::instance().print_nodeaddr(iph_ack->saddr()), iph_ack->sport());
	debug("At %f NEMO Agent in %s send binding update ack message using interface %s \n", 
			NOW, MYNUM, Address::instance().print_nodeaddr(iface_->address()));
	
	send(p_ack, 0);
}

