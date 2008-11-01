/*
 * Implementation of interface management protocol
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

#include "mip6.h"
#include "nd.h"
#include "handover.h"
//NC
#include "mac-802_11.h"

#define MYNUM	Address::instance().print_nodeaddr(addr())

/****************************************
 implementation of NEMO message
 2003/9/26 hscho@mmlab.snu.ac.kr
 ****************************************/
int hdr_nemo::offset_;
static class NEMOHeaderClass : public PacketHeaderClass {
public:
	NEMOHeaderClass() :
		PacketHeaderClass("PacketHeader/NEMO", sizeof(hdr_nemo)) {
		bind_offset(&hdr_nemo::offset_);
	}
} class_nemohdr;

int hdr_tunnel::offset_;
static class TUNNELHeaderClass : public PacketHeaderClass {
public:
	TUNNELHeaderClass() :
		PacketHeaderClass("PacketHeader/TUNNEL", sizeof(hdr_tunnel)) {
		bind_offset(&hdr_tunnel::offset_);
	}
} class_tunnelhdr;

/*
 * Packet definitions
 */
int hdr_rtred::offset_;
static class RTREDHeaderClass : public PacketHeaderClass {
public:
	RTREDHeaderClass() :
		PacketHeaderClass("PacketHeader/RTRED", sizeof(hdr_rtred)) {
		bind_offset(&hdr_rtred::offset_);
	}
} class_rtredhdr;

int hdr_freq::offset_;
static class FREQHeaderClass : public PacketHeaderClass {
public:
	FREQHeaderClass() :
		PacketHeaderClass("PacketHeader/FREQ", sizeof(hdr_freq)) {
		bind_offset(&hdr_freq::offset_);
	}
} class_freqhdr;

int hdr_fres::offset_;
static class FRESHeaderClass : public PacketHeaderClass {
public:
	FRESHeaderClass() :
		PacketHeaderClass("PacketHeader/FRES", sizeof(hdr_fres)) {
		bind_offset(&hdr_fres::offset_);
	}
} class_freshdr;

int hdr_hand::offset_;
static class HANDHeaderClass : public PacketHeaderClass {
public:
	HANDHeaderClass() :
		PacketHeaderClass("PacketHeader/HAND", sizeof(hdr_hand)) {
		bind_offset(&hdr_hand::offset_);
	}
} class_handhdr;

/*
 * Static constructor for TCL
 */
static class MIPV6AgentClass : public TclClass {
public:
	MIPV6AgentClass() :
		TclClass("Agent/MIHUser/IFMNGMT/MIPV6") {
	}
	TclObject* create(int, const char*const*) {
		return (new MIPV6Agent());

	}
} class_ifmngmtagent;

/*
 * Creates a neighbor discovery agent
 */
MIPV6Agent::MIPV6Agent() : IFMNGMTAgent(), udpmysip_(NULL) {
	LIST_INIT(&bulist_head_);
//hoa_(0), ha_(0), coa_(0), nemo_prefix_(0), iface_node_(NULL)
//	LIST_INIT(&tunnel_head_);
//	LIST_INIT(&bslist_head_);
//	LIST_INIT(&history_head_);
	flowRequestTimer_ = new FlowRequestTimer (this);
	seqNumberFlowRequest_ = 0;
	bind("exp_", &exp_);
	bind("flowRequestTimeout_", &flowRequestTimeout_);
}

/* 
 * Interface with TCL interpreter
 * @param argc The number of elements in argv
 * @param argv The list of arguments
 * @return TCL_OK if everything went well else TCL_ERROR
 */
int MIPV6Agent::command(int argc, const char*const* argv) {
	//----------------sem start------------------//
	if (argc==2)
	{
		if (strcmp(argv[1], "binding")==0)
		{
			send_bu_msg();
			return TCL_OK;
		}
	}
	if (argc==3)
	{
//		if(strcmp(argv[1], "connect-nemo")==0) {
//			nemo_ = (NEMOAgent *)TclObject::lookup(argv[2]);
//			if(nemo_)
//				return TCL_OK;
//			return TCL_ERROR;
//		}
		
		if (strcmp(argv[1], "set-node-type")==0)
		{
			switch (atoi(argv[2]))
			{
					case MN:
						node_type_ = MN;
						break;
					case MN_HA:
						node_type_ =  MN_HA;
						break;
					case MR:
						node_type_ = MR;
						break;
					case MR_HA:
						node_type_ = MR_HA;
						break;
					case CN:
						node_type_ = CN;
						break;
					default:
						debug("no match type\n");
						return TCL_ERROR;
						break;
			}
			debug("node_type_ %d\n",node_type_);
			return TCL_OK;
		}
//		if (strcmp(argv[1], "binding")==0)
//		{
//			iface_node_ = (Node *) TclObject::lookup(argv[2]);
//			send_bu_msg(iface_node_);
//			return TCL_OK;
//		}
//		if (strcmp(argv[1], "mipv6-interface")==0)
//		{
//			Tcl& tcl = Tcl::instance();
//			tcl.evalf("%s target [%s entry]", this->name(), argv[2]);
//			return TCL_OK;
//		}
		if (strcmp(argv[1], "set-udpmysip")==0)
		{
			udpmysip_= (UdpmysipAgent *) TclObject::lookup(argv[2]);
			if(udpmysip_==0)
				return TCL_ERROR;
			return TCL_OK;
		}
//		if (strcmp(argv[1], "set-nemo-prefix")==0)
//		{
//			nemo_prefix_ = Address::instance().str2addr(argv[2]);	
//			if(nemo_prefix_==0)
//				return TCL_ERROR;
//			return TCL_OK;
//		}
	}
	if (argc==4) 
	{
		if (strcmp(argv[1], "set-cn")==0) 
		{
			int ha_ = Address::instance().str2addr(argv[2]);	//home agent address
			int hoa_ = Address::instance().str2addr(argv[3]);	//home address
			BUEntry* bu = new BUEntry(MN_HA);
			bu->update_entry(ha_, hoa_);
			bu->insert_entry(&bulist_head_);
			dump();
			return TCL_OK;
		}
	}
	if (argc==5) 
		{
			if (strcmp(argv[1], "set-mn")==0) 
			{
				int ha_ = Address::instance().str2addr(argv[2]);														//	home agent address
				int hoa_ = Address::instance().str2addr(argv[3]);														//	home address
				NEMOAgent *eface_agent_ = (NEMOAgent *)TclObject::lookup(argv[4]);		// external interface nemo agent
				if (eface_agent_==NULL)
					return TCL_ERROR;
				BUEntry* bu = new BUEntry(MN_HA);
				bu->update_entry(ha_, hoa_, eface_agent_);
				bu->insert_entry(&bulist_head_);
				dump();
				return TCL_OK;
			}
		}
	if (argc==7)
	{
		if (strcmp(argv[1], "set-mr")==0) 
		{
			int ha_ = Address::instance().str2addr(argv[2]);													//	home agent address
			int hoa_ = Address::instance().str2addr(argv[3]);													//	home address
			int nemo_prefix_ = Address::instance().str2addr(argv[4]);							//	nemo prefix
			NEMOAgent *eface_agent_ = (NEMOAgent *)TclObject::lookup(argv[5]);		// external interface nemo agent
			NEMOAgent *iface_agent_ = (NEMOAgent *)TclObject::lookup(argv[6]);		// internal interface nemo agent
			if (eface_agent_==NULL && iface_agent_==NULL)
				return TCL_ERROR;
			BUEntry* bu = new BUEntry(MR_HA);
			bu->update_entry(ha_, hoa_, nemo_prefix_, eface_agent_, iface_agent_);
			bu->insert_entry(&bulist_head_);
			dump();
			return TCL_OK;
		}
	}
	//----------------sem end------------------//
	return (IFMNGMTAgent::command(argc, argv));
}

/*
 * Process the given event
 * @param type The event type
 * @param data The data associated with the event
 */
void MIPV6Agent::process_nd_event(int type, void* data) {
	switch (type) {
	case ND_NEW_PREFIX:
		process_new_prefix((new_prefix *)data);
		break;
	case ND_PREFIX_EXPIRED:
		process_exp_prefix((exp_prefix *)data);
		break;
	}
}

/* 
 * Process received packet
 * @param p The received packet
 * @param h Packet handler
 */
void MIPV6Agent::recv(Packet* p, Handler *h) {
	hdr_cmn *hdrc= HDR_CMN(p);

	switch (hdrc->ptype()) {
	case PT_RRED:
		recv_redirect(p);
		// handover_->recv (p,h); 
		break;
	case PT_FREQ:
		recv_flow_request(p);
		break;
	case PT_FRES:
		recv_flow_response(p);
		break;
	case PT_HAND:
		//handover_->recv (p, h);
		break;
	case PT_NEMO:
		recv_nemo(p);
		break;
	default:
		debug("At %f MIPv6 Agent in %s received Unknown packet type\n", 
		NOW, MYNUM);
		tunneling(p);
	}
	Packet::free(p);
}

/*
 * Send an update message to the interface management.
 * @param dagent The agent that needs to redirect its messages
 * @param iface The new interface to use
 */
void MIPV6Agent::send_update_msg(Agent *dagent, Node *iface) {
	vector <Agent *> agents;
	agents.push_back(dagent);
	send_update_msg(agents, iface);
}

/*
 * Send an update message to the interface management.
 * @param dagent The agent that needs to redirect its messages
 * @param iface The new interface to use
 */
void MIPV6Agent::send_update_msg(vector <Agent *> dagent, Node *iface) {
	Packet *p = allocpkt();
	hdr_ip *iph= HDR_IP(p);
	hdr_rtred *rh= HDR_RTRED(p);
	hdr_cmn *hdrc= HDR_CMN(p);

	//----------------sem start------------------//
	//	hdr_nemo *nh= HDR_NEMO(p);
	//	nh->haddr() = HADDR;
	//	nh->H()=ON;
	//	nh->A()=ON;

	//----------------sem end------------------//


	iph->saddr() = iface->address(); //we overwrite to use the interface address
	iph->daddr() = dagent.at(0)->addr();
	//	printf("%d addr()\n",dagent.at(0)->addr());
	//	iph->daddr() = ha_;
	//	printf("%d addr()\n",ha_);
	iph->dport() = port();
	hdrc->ptype() = PT_RRED;
	//	hdrc->ptype() = PT_NEMO;
	hdrc->size() = PT_RRED_SIZE;

	rh->ack() = 0;
	rh->destination() = iface->address();
	for (int i = 0; i < (int)dagent.size(); i++)
		rh->agent().push_back(dagent.at(i));

	//we need to send using the interface
	{
		Tcl& tcl = Tcl::instance();
		tcl.evalf("%s target [%s entry]", this->name(), iface->name());
	}

	debug("At %f MIPv6 Agent in %s send redirect message using interface %s\n", 
	NOW, MYNUM, Address::instance().print_nodeaddr(iface->address()));

	send(p, 0);

}

/* 
 * Process received packet
 * @param p The received packet
 */
void MIPV6Agent::recv_redirect(Packet* p) {
	//received packet
	hdr_ip *iph= HDR_IP(p);
	hdr_rtred *rh= HDR_RTRED(p);

	if (rh->ack()) {
		recv_redirect_ack(p);
		return;
	}

	//reply packet
	Packet *p_ack = allocpkt();
	hdr_ip *iph_ack= HDR_IP(p_ack);
	hdr_rtred *rh_ack= HDR_RTRED(p_ack);
	hdr_cmn *hdrc_ack= HDR_CMN(p_ack);

	Tcl& tcl = Tcl::instance();

	assert (HDR_CMN(p)->ptype() == PT_RRED);

	debug("At %f MIPv6 Agent in %s received redirect packet from %s\n", 
	NOW, MYNUM, Address::instance().print_nodeaddr(iph->saddr()));

	for (int i = 0; i < (int) rh->agent().size() ; i++) {
		//modify the destination address of the agent.
		tcl.evalf("%s set dst_addr_ %d", rh->agent().at(i)->name(), rh->destination());
	}

	//send ack message to sender
	iph_ack->daddr() = iph->saddr();
	iph_ack->dport() = iph->sport();
	hdrc_ack->ptype() = PT_RRED;
	hdrc_ack->size() = PT_RRED_SIZE;

	rh_ack->ack() = 1;
	rh_ack->destination() = rh->destination();
	rh_ack->agent() = rh->agent();

	send(p_ack, 0);
}

/* 
 * Process received packet
 * @param p The received packet
 */
void MIPV6Agent::recv_redirect_ack(Packet* p) {
	//received packet
	hdr_ip *iph= HDR_IP(p);

	assert (HDR_CMN(p)->ptype() == PT_RRED);

	debug("At %f MIPv6 Agent in %s received ack for redirect packet from %s\n", 
	NOW, MYNUM, Address::instance().print_nodeaddr(iph->saddr()));

	//do additional processing ??
}

/*
 * Send an RS message for the given interface
 * @param mac The interface to use
 */
void MIPV6Agent::send_rs(Mac *mac) {
	assert (mac);

	NDAgent *nd_agent = get_nd_by_mac(mac);
	if (nd_agent) {
		debug("At %f MIPv6 Agent in %s requests ND to send RS\n", NOW, MYNUM);
		nd_agent->send_rs();
	} else {
		debug("WARNING: At %f MIPv6 Agent in %s does not have ND to send RS\n", 
		NOW, MYNUM);
	}

}

void MIPV6Agent::send_flow_request(vector<FlowEntry*> flows, Node *iface,
		int destination) {
	Packet *p = allocpkt();
	hdr_ip *iph= HDR_IP(p);
	hdr_freq *rh= HDR_FREQ(p);
	hdr_cmn *hdrc= HDR_CMN(p);

	iph->daddr() = destination;
	iph->saddr() = iface->address();
	iph->dport() = port();
	hdrc->ptype() = PT_FREQ;
	hdrc->size() = PT_FREQ_SIZE;

	debug("At %f MIPv6 Agent in %s send flow request to %s\n", 
	NOW, MYNUM, Address::instance().print_nodeaddr(destination));

	rh->flow_info = flows;
	//seqNumberFlowRequest_ ++;
	rh->seqNumber_ = seqNumberFlowRequest_;
	//we need to send using the interface
	{
		Tcl& tcl = Tcl::instance();
		tcl.evalf("%s target [%s entry]", this->name(), iface->name());
	}
	flowRequestTimer_->attach_event(flows, iface, destination);
	flowRequestTimer_->resched(flowRequestTimeout_);
	send(p, 0);
}

/* 
 * Process received packet
 * @param p The received packet
 */
void MIPV6Agent::recv_flow_request(Packet* p) {
	//received packet
	hdr_ip *iph= HDR_IP(p);
	hdr_freq *rh= HDR_FREQ(p);
	//repsonse packet
	Packet *p_res = allocpkt();
	hdr_ip *iph_res= HDR_IP(p_res);
	hdr_fres *rh_res= HDR_FRES(p_res);
	hdr_cmn *hdrc_res= HDR_CMN(p_res);

	float requested_bw=0;
	float available_bw = 5000;

	assert (HDR_CMN(p)->ptype() == PT_FREQ);

	debug("At %f MIPv6 Agent in %s received flow request packet\n", NOW, MYNUM);

	//We check if we can accept the client

	//compute the total requested bandwidth

	for (int i= 0; i < (int) (rh->flow_info.size()) ; i++) {
		requested_bw += rh->flow_info.at(i)->minBw();
	}
	debug("\tRequested bandwidth = %f. Total aleady used=%f, available=%f \n",
			requested_bw, get_used_bw(), available_bw);

	//NC: get the bssid of the BS
	Tcl& tcl = Tcl::instance();
	tcl.evalf("%s set mac_(0)", rh->flow_info.at(0)->tmp_iface->name());
	Mac *mac = (Mac*) TclObject::lookup(tcl.result());
	//printf("bss_id %d \n",((Mac802_11*)(mac))->bss_id());

	if ((((Mac802_11*) (mac))->bss_id()) != 8) {
		//end NC
		//if ((available_bw-get_used_bw()) >= requested_bw){
		//we accept the new flows
		for (int i= 0; i < (int) (rh->flow_info.size()) ; i++) {
			list_node *new_info = (list_node*) malloc(sizeof(list_node));
			rh->flow_info.at(i)->redirect_ = true;
			//store information
			new_info->minBw= rh->flow_info.at(i)->minBw();
			new_info->node = rh->flow_info.at(i)->tmp_iface;
			list_nodes.push_back(new_info);
			//update file
			nbAccepted_++;
			//fprintf(Mac802_11::finfocom,"Connection accepted \t A %i \t R %i \t Bw %f\n",nbAccepted,nbRefused, get_used_bw());
			//fflush(Mac802_11::finfocom);
		}
	} else {
		for (int i= 0; i < (int) (rh->flow_info.size()) ; i++) {
			nbRefused_++;
			//NC
			rh->flow_info.at(i)->redirect_ = false;
			//end NC
			//fprintf(Mac802_11::finfocom,"Connection refused \t A %i \t R %i \t Bw %f\n",
			//      nbAccepted_, nbRefused_, get_used_bw()); 
			//fflush(Mac802_11::finfocom);
		}
	}

	//send result
	rh_res->flow_info = rh->flow_info; //attach same data
	iph_res->daddr() = iph->saddr();
	iph_res->dport() = iph->sport();
	hdrc_res->ptype() = PT_FRES;
	hdrc_res->size() = PT_FRES_SIZE;

	rh_res->seqNumber_ = rh->seqNumber_;
	debug("\tSending reply to %s\n", Address::instance().print_nodeaddr((int)(iph_res->daddr())));
	send(p_res, 0);

	/*802.21*/
	//tell the handover there is a new remote MAC
	//we want to register for link going down
	//get the mac of the node we need to connect
	/*
	 if (handover_) {
	 Tcl& tcl = Tcl::instance();
	 tcl.evalf ("%s set mac_(0)", rh->flow_info.at(0)->tmp_iface->name());
	 Mac *mac = (Mac*) TclObject::lookup(tcl.result());
	 handover_->register_remote_mih (mac);
	 }
	 */
}

/* 
 * Process received packet
 * @param p The received packet
 */
void MIPV6Agent::recv_flow_response(Packet* p) {
	hdr_fres *rh= HDR_FRES(p);
	Tcl& tcl = Tcl::instance();
	//NC
	int flow_accepted = 0;
	//end NC

	assert (HDR_CMN(p)->ptype() == PT_FRES);

	debug("At %f MIPv6 Agent in %s received flow response packet\n", NOW, MYNUM);
	// cancel flow resquet timer
	if (flowRequestTimer_->status()!=TIMER_IDLE) {
		flowRequestTimer_->cancel();
	}

	if (seqNumberFlowRequest_ == rh->seqNumber_) {
		//we look at the answer and redirect for the accepted flows
		for (int i= 0; i < (int) (rh->flow_info.size()) ; i++) {
			if (rh->flow_info.at(i)->redirect_) {
				debug("\tflow %d accepted...we redirect\n", i);
				tcl.evalf("%s target [%s entry]", rh->flow_info.at(i)->local()->name(), rh->flow_info.at(i)->tmp_iface->name());
				vector<Agent*> agents;
				agents.push_back(rh->flow_info.at(i)->remote());
				send_update_msg(agents, rh->flow_info.at(i)->tmp_iface);
				rh->flow_info.at(i)->update_interface(rh->flow_info.at(i)->tmp_iface);
				//NC: If one flow is accepted the interface has to stay up
				flow_accepted = 1;
				//end NC
			} else {
				//debug ("\tflow %d refused.\n",i);
				//tcl.evalf ("%s set X_ 10", rh->flow_info.at(i)->tmp_iface->name());
				//tcl.evalf ("%s set Y_ 10", rh->flow_info.at(i)->tmp_iface->name());
				//tcl.evalf ("[%s set ragent_] set perup_ 1000",rh->flow_info.at(i)->tmp_iface->name() );
			}
		}
		//NC: if all the flows were rejected, disconnect the interface
		/*
		 if( flow_accepted == 0){
		 Tcl& tcl = Tcl::instance();
		 tcl.evalf ("%s set mac_(0)", rh->flow_info.at(0)->tmp_iface->name());
		 Mac *mac = (Mac*) TclObject::lookup(tcl.result());
		 mih_->link_disconnect(mac);
		 //end NC
		 }
		 */
		seqNumberFlowRequest_ ++;
	}
}

/* 
 * Compute the bandwidth used by registered MNs
 * @return The bandwidth used by registered MNs
 */
float MIPV6Agent::get_used_bw() {
	float used_bw=0;

	for (int i = 0; i < (int)list_nodes.size() ; i++) {
		used_bw += list_nodes.at(i)->minBw;
	}

	return used_bw;
}

/*
 void MIPV6Agent::process_client_going_down (int macSrc)
 {
 Tcl& tcl = Tcl::instance();

 if (print_info_)
 cout << "At " << NOW << " MIPv6 agent in node " << MYNUM 
 << " received client going down\n";

 //we check if we find the structure 
 for (int i=0; i < (int)list_nodes.size() ; i++) {
 tcl.evalf ("%s get-mac-by-addr %d", list_nodes.at(i)->node->name(),macSrc);
 if (tcl.result()!="") {
 list_node *tmp = list_nodes.at (i);
 printf ("Found node %s to remove\n", Address::instance().print_nodeaddr(list_nodes.at(i)->node->address()));
 list_nodes.erase (list_nodes.begin()+i);
 free (tmp);
 break; 
 }
 }
 //Nist - infocom
 //fprintf(Mac802_11::finfocom, "%f - %i - Deconnexion\n", NOW, macSrc);

 }
 */

/*
 * Process a new prefix entry
 * @param data The new prefix information 
 */
void MIPV6Agent::process_new_prefix(new_prefix* data) {
	//to be defined by subclass
	//----------------sem start------------------//
	printf("MIPv6Agent::process_new_prefix\n");
	compute_new_address (data->prefix, data->interface);
	if(udpmysip_!=0 )
	{
		printf("sip enable\n");
		udpmysip_->send_reg_msg(data->prefix, data->interface);
	}
	else	
		send_bu_msg(data->prefix, data->interface);
	//----------------sem end------------------//
	
	free(data);
}

/*
 * Process an expired prefix
 * @param data The information about the prefix that expired
 */
void MIPV6Agent::process_exp_prefix(exp_prefix* data) {
	//to be defined by subclass
	printf("MIPv6Agent::process_exp_prefix\n");
	
	free(data);
}



void MIPV6Agent::dump() {
	
//	BUEntry* node = head(bulist_head_);
	BUEntry *node =  bulist_head_.lh_first;
	
	if (node) {
		
//		debug("\n|%s for node %s at %d\n",txt,PRINTADDR(addr()),NOW);
//		debug("|Node \t HoA \t CoA \t Type \t Info\t	Mac\n");
//		for (; node; node=node->next_entry() ) {
//			debug("|%s \t %s \t %s \t %d \t %s \t %d \n", PRINTADDR(node->addr), PRINTADDR(node->haddr), PRINTADDR(node->caddr),
//					node->type, node->info, node->mac_->addr());
//		}
		
		cout <<"\n|"<< "Binding Update List" << " for node "<< PRINTADDR(addr()) <<" at "<< NOW <<"\n";
		cout <<"|Type\tAddr \t Haddr \t Caddr \t NEMO_prefixe \t eFace \t iFace \n";

		for (; node; node=node->next_entry() ) {
			if(node->eface()!=NULL && node->iface()!=NULL) {
				
				cout <<" | "<< node->type() <<" \t \t " << PRINTADDR(node->addr()) <<"\t" 
					<< PRINTADDR(node->haddr()) <<"\t"<< PRINTADDR(node->caddr()) <<"\t"
					<< PRINTADDR(node->prefix()) << "\t"
					<< PRINTADDR(node->eface()->get_iface()->address()) << "\t" 
					<< PRINTADDR(node->iface()->get_iface()->address()) << "\n";
				
			} else {
				
				cout <<" | "<< node->type() <<" \t \t " << PRINTADDR(node->addr()) <<"\t" 
					<< PRINTADDR(node->haddr()) <<"\t"<< PRINTADDR(node->caddr()) <<"\t"
					<< PRINTADDR(node->prefix()) << "\n";
				
			}
		}

		cout << "\n";
	}
}

//BUEntry* MIPV6Agent::lookup_coa(int addr, BUEntry* n) {
//	for (; n; n=n->next_entry() ) 
//	{
//		if (n->addr() == addr) 
//		{
//			return n;
//		}
//	}
//	return NULL;
//}
//
//BUEntry* MIPV6Agent::lookup_hoa(int addr, BUEntry* n) {
//	for (; n; n=n->next_entry() ) 
//	{
//		if (n->caddr() == addr) 
//		{
//			return n;
//		}
//	}
//	return NULL;
//}
//
//BUEntry* MIPV6Agent::lookup_entry(int addr, int coa, BUEntry* n) {
//	for ( ; n ; n=n->next_entry() )
//	{
//		if ( n->addr() == addr && n->caddr() == coa )
//		{
//			return n;
//		}
//	}
//	return NULL;
//}

void MIPV6Agent::recv_nemo(Packet* p) {
		
	debug("At %f MIPv6 Agent in %s received nemo packet\n", NOW, MYNUM);
	
	hdr_ip *iph= HDR_IP(p);
	hdr_nemo* nh= HDR_NEMO(p);
	
	if(node_type_==MR)
	{
		//debug("MIPv6 MR\n");
		
		
		if(nh->type()==BU) 
		{
			delete_tunnel(p);
			BUEntry* bu = get_entry_by_addr(iph->daddr());
			if(!bu)
			{
				//	can not find in the binding list
				//	-> MN send to me or MN send to MN-HA
					
				if (nh->H()==ON) 
				{
					//	mn has ha, register myself and tunnel to mr-ha
					
					//delete_tunnel(p);
					//registration(p);
					//dump();
					
					BUEntry* bu = get_entry_by_haddr(nh->haddr());
					if(!bu)
					{
						registration(p,MN);
						dump();
					}
					
					//	choise mn entry
					BUEntry* mn = get_entry_by_haddr(nh->haddr());
	
					int prefix = iph->saddr() & 0xFFFFF800;
					
					bu= get_entry_by_prefix(prefix);
					
					assert(bu!=NULL);
					
					//	change mn entry coa to outside coa
					//	change mn entry eface to outdise eface
					mn->caddr() = bu->caddr();
					mn->eface_agent_ = bu->eface_agent_;
					dump();
					
					//	tunnel packet to MR-HA
					add_tunnel(p);
					iph->daddr()=bu->addr();
					
					Packet* p_tunnel = p->copy();
					bu->eface()->send(p_tunnel,0);

					debug("At %f MIPv6 Agent in %s tunnel nemo packet to %s\n", 
									NOW, MYNUM, Address::instance().print_nodeaddr(iph->daddr()));
					
				} else {
					//	mn has no ha, register myself
					registration(p, MN);
					dump();
					if (nh->A()==ON) 
					{
						int prefix = iph->saddr() & 0xFFFFF800;
						debug ("prefix=%s \n", Address::instance().print_nodeaddr(prefix));
						get_iface_agent_by_prefix(prefix)->send_bu_ack(p);
					}
				}
			} else {
							debug("bu true\n");
							//	if daddr is MN in the binding list -> forward to MN
							int prefix = iph->daddr() & 0xFFFFF800;
							debug("prefix= %s \n", Address::instance().print_nodeaddr(prefix));
							Packet* p_tunnel = p->copy();
							get_iface_agent_by_prefix(prefix)->send(p_tunnel,0);
						}
		}
		else if (nh->type()==BACK) 
		{
			BUEntry* bu = get_entry_by_caddr(iph->daddr());
			
			if(!bu)
			{
				//	daddr is others
				bu = get_entry_by_addr(iph->saddr());
				Packet* p_tunnel = p->copy();
				bu->eface()->send(p_tunnel,0);
				
			} else {
				if(size_tunnel(p)==0)
				{					
					//	daddr is me
					recv_bu_ack(p);
				} else {
					//	daddr is nemo prefix
					delete_tunnel(p);
					int prefix = iph->daddr() & 0xFFFFF800;
					debug("prefix=%s \n",Address::instance().print_nodeaddr(prefix));
//					BUEntry* bu = get_entry_by_prefix(prefix);
					Packet* p_tunnel = p->copy();
					get_iface_agent_by_prefix(prefix)->send(p_tunnel,0);
				}
			}
			
		}
		
	} 
	else if (node_type_==MR_HA)
	{
		//debug("MIPv6 MR_HA\n");
		if(nh->type()==BU) 
		{
				delete_tunnel(p);
				debug("MR_HA size_tunnel %d\n",size_tunnel(p));
				
				if(iph->daddr()==addr()) {
					//	if	MR_HA recv BU to me
					registration(p,MR);
					dump();
					debug("At %f MIPv6 MR_HA Agent in %s recv BU packet\n", NOW, MYNUM);
					if (nh->A()==ON) 
					{
						send_bu_ack(p);
					}
					
				} else {
					
				
					int prefix = iph->daddr() & 0xFFFFF800;
					debug("prefix=%s \n",Address::instance().print_nodeaddr(prefix));
					BUEntry* bu = get_entry_by_prefix(prefix);
					if(!bu)
					{
						int tunnel_num = size_tunnel(p);
						
						if(tunnel_num==0){
							//	if MR_HA recv BU from MN to MN-HA -> change saddr to MR_HA
							iph->saddr()=addr();
							
						} else {
							//	If MR_HA recv BU from MN to MN
							int daddr = iph->daddr();
							delete_tunnel(p);
							iph->saddr()=addr();
							add_tunnel(p);
							iph->daddr()=daddr;
						}

						printf("MR_HA test %s %d \n",Address::instance().print_nodeaddr(iph->daddr()),tunnel_num);
					} else {
						
						//	if	MR_HA recv BU to MN -> forward to MR
						add_tunnel(p);
						iph->daddr() = bu->caddr();
						
					}
					
						
					Packet* p_untunnel = p->copy();
					send(p_untunnel,0);
					debug("At %f MIPv6 MR_HA Agent in %s send BU packet to MN_HA or CN or MN %s\n", 
							NOW, MYNUM, Address::instance().print_nodeaddr(iph->daddr()));
				}

		} else if (nh->type()==BACK) 
		{
			if(iph->daddr()==addr())
			{
				if(size_tunnel(p)==0)
				{
					//	daddr is me
					recv_bu_ack(p);
				} else {
					//	daddr is nemo node
					delete_tunnel(p);
					int prefix = iph->daddr() & 0xFFFFF800;
					debug("prefix=%s \n",Address::instance().print_nodeaddr(prefix));
					BUEntry* bu = get_entry_by_prefix(prefix);
					add_tunnel(p);
					iph->daddr()=bu->caddr();
					Packet* p_tunnel = p->copy();
					send(p_tunnel,0);
				}

			} else {
					//	daddr is other node
					Packet* p_copy = p->copy();
					send(p_copy,0);
			}
			
//			if(!bu)
//			debug("nh->haddr() %s", Address::instance().print_nodeaddr(nh->haddr()));
//			BUEntry* bu = get_entry_by_haddr(nh->haddr());
//			if(!bu)
//			{
//				//	if not my HoA, send back to MR
//				bu = get_entry_by_haddr(nh->coa());
//				iph->daddr()=bu->caddr();
//				Packet* p_tunnel = p->copy();
//				send(p_tunnel,0);
//			}else
//					recv_bu_ack(p);
			
		} else if (nh->type()==REHOME)
		{
			delete_tunnel(p);
			BUEntry* bu = get_entry_by_haddr(nh->haddr());
			assert(bu!=NULL);
			bu->caddr()=nh->coa();
			
			nh->haddr()=nh->coa();
			nh->coa()=iph->saddr();
			registration(p, MR);
			dump();
			
			debug("At %f MIPv6 MR_HA Agent in %s recv REHOME packet\n", NOW, MYNUM);
//								if (nh->A()==ON) 
//								{
//									send_bu_ack(p);
//								}
		}
		
	}
	else if (node_type_==MN_HA)
	{
		debug("MIPv6 MN_HA\n");
		if(nh->type()==BU) 
		{
			if (nh->H()==ON) 
			{
				registration(p,MN);
				dump();
				debug("At %f MIPv6 MN_HA Agent in %s recv BU packet\n", NOW, MYNUM);
			}
			if (nh->A()==ON) 
			{
				send_bu_ack(p);
			}
		} else if (nh->type()==BACK) 
		{
			recv_bu_ack(p);
		}
	}
	else if (node_type_==MN)
	{
		debug("MIPv6 MN\n");
		if(nh->type()==BU) 
		{
			if (nh->H()==ON) 
			{
				registration(p,MN);
				dump();
				debug("At %f MIPv6 MN Agent in %s recv BU packet\n", NOW, MYNUM);
			}
//			if (nh->A()==ON) 
//			{
//				send_bu_ack(p);
//			}
		} else if (nh->type()==BACK) 
		{
			recv_bu_ack(p);
		}
	}
	else if (node_type_==CN)
	{
		debug("MIPv6 CN\n");
		if(nh->type()==BU) 
		{
			if (nh->H()==ON) 
			{
				registration(p,MN);
				dump();
				debug("At %f MIPv6 CN Agent in %s recv BU packet\n", NOW, MYNUM);
			}
			if (nh->A()==ON) 
			{
				send_bu_ack(p);
			}
		} else if (nh->type()==BACK) 
		{
			recv_bu_ack(p);
		}
	}

}

void MIPV6Agent::registration(Packet* p, Mipv6NodeType type) {
	hdr_ip *iph= HDR_IP(p);
	hdr_nemo* nh= HDR_NEMO(p);
	
	
	printf("MIPV6Agent::registration hdr_nemo---> coa %s haddr %s r %d\n",
			Address::instance().print_nodeaddr(nh->coa()), Address::instance().print_nodeaddr(nh->haddr()), nh->R() );
	
	if(type==MR)
	{
		BUEntry* bu = get_entry_by_haddr(nh->haddr());
		if(!bu)
		{
			bu = new BUEntry(MR);
			bu->update_entry(nh->haddr(),nh->coa(),nh->nemo_prefix());
			bu->insert_entry(&bulist_head_);
		}
		bu->caddr()=nh->coa();
		
	} 
	else if (type==MN)
	{
		BUEntry* bu = get_entry_by_haddr(nh->haddr());
		if(!bu)
		{
			bu = new BUEntry(MN);
			bu->addr()=iph->saddr();
			bu->haddr()=nh->haddr();
			bu->caddr()=nh->coa();
			bu->insert_entry(&bulist_head_);
		}
		bu->caddr()=nh->coa();
	}
	else if (type==CN)
	{
		
	}
	
}

void MIPV6Agent::send_bu_msg() {
	
	Packet *p = allocpkt();
	hdr_ip *iph= HDR_IP(p);
	hdr_cmn *hdrc= HDR_CMN(p);
	
	BUEntry *bu =  bulist_head_.lh_first;
	assert(bu!=NULL);
	
	//----------------sem start------------------//
	hdr_nemo *nh= HDR_NEMO(p);
	nh->haddr() = bu->haddr();
	nh->coa()=addr();
	nh->H()=ON;
	nh->A()=ON;
	nh->type()=BU;
	nh->lifetime()=TIME_INFINITY;
	nh->seqno()=0;
	//----------------sem end------------------//

	iph->saddr() = addr(); //we overwrite to use the interface address
	iph->daddr() = bu->addr();
	iph->dport() = port();
	hdrc->ptype() = PT_NEMO;
	hdrc->size() = IPv6_HEADER_SIZE + BU_SIZE;
	
	add_tunnel(p);
	debug("At %f MIPv6 Agent in %s send binding update\n", NOW, MYNUM);

	send(p, 0);

}

void MIPV6Agent::send_bu_msg(int prefix, Node *iface) {
	
	BUEntry *bu =  get_entry_by_iface(iface);
	assert(bu!=NULL);

//	debug(
//				"prefix %s iface->address %s\n", Address::instance().print_nodeaddr(prefix),
//				Address::instance().print_nodeaddr(iface->address()));

	
	bu->caddr() = iface->address();
	
	Packet *p = allocpkt();
	hdr_ip *iph= HDR_IP(p);
	hdr_cmn *hdrc= HDR_CMN(p);
	hdr_nemo *nh= HDR_NEMO(p);
	
//	if(node_type_==MN)
//	{
//		
//	}
//	if(bu!=NULL) {
//		debug("MN node \n");
//		nh->coa()=iface->address();
//		nh->haddr() = bu->haddr;	
//		nh->H()=ON;
//		nh->A()=ON;
//		nh->type()=BU;
//		nh->lifetime()=TIME_INFINITY;
//		nh->seqno()=0;
//		
//		iph->saddr() = iface->address();
//		iph->daddr()= bu->addr	;
//		iph->dport() = port();
//		hdrc->ptype() = PT_NEMO;
//		hdrc->size() = IPv6_HEADER_SIZE + BU_SIZE;
//		
//		add_tunnel(p);				
//		iph->daddr() = prefix;
//		
//	} else {
//		debug("LFN node \n");
//		nh->coa()=iface->address();
//		nh->haddr() = prefix;
//		nh->H()=OFF;
//		nh->A()=ON;
//		nh->type()=BU;
//		nh->lifetime()=TIME_INFINITY;
//		nh->seqno()=0;
//		
//		iph->saddr() = iface->address();
//		iph->daddr() = prefix;
//		iph->dport() = port();
//		hdrc->ptype() = PT_NEMO;
//		hdrc->size() = IPv6_HEADER_SIZE + BU_SIZE;
//		
//	}
//	
//	bu->eface()->send(p,0);
	
	
	if(node_type_==MR)
	{
		debug("MR node BU \n");
		nh->coa()=iface->address();
		nh->haddr() = bu->haddr();	
		nh->H()=ON;
		nh->A()=ON;
		nh->type()=BU;
		nh->lifetime()=TIME_INFINITY;
		nh->seqno()=0;
		nh->nemo_prefix()=bu->prefix();
		
		iph->saddr() = iface->address();
		iph->daddr()= bu->addr();
		iph->dport() = port();
		hdrc->ptype() = PT_NEMO;
		hdrc->size() = IPv6_HEADER_SIZE + BU_SIZE;
		
		add_tunnel(p);	
		
		Tcl& tcl = Tcl::instance();
		tcl.evalf("%s entry", iface->name());
		NsObject* obj = (NsObject*) TclObject::lookup(tcl.result());
		Scheduler::instance().schedule(obj,p->copy(),0.1);
	
	} else {
	
		if(bu!=NULL) {
				debug("MN node BU \n");
				nh->coa()=iface->address();
				nh->haddr() = bu->haddr();	
				nh->H()=ON;
				nh->A()=ON;
				nh->type()=BU;
				nh->lifetime()=TIME_INFINITY;
				nh->seqno()=0;
				
				iph->saddr() = iface->address();
				iph->daddr()= bu->addr();
				iph->dport() = port();
				hdrc->ptype() = PT_NEMO;
				hdrc->size() = IPv6_HEADER_SIZE + BU_SIZE;
				
			} else {
				
				debug("LFN node BU \n");
				nh->coa()=iface->address();
				nh->haddr() = prefix;
				nh->H()=OFF;
				nh->A()=ON;
				nh->type()=BU;
				nh->lifetime()=TIME_INFINITY;
				nh->seqno()=0;
				
				iph->saddr() = iface->address();
				iph->daddr() = prefix;
				iph->dport() = port();
				hdrc->ptype() = PT_NEMO;
				hdrc->size() = IPv6_HEADER_SIZE + BU_SIZE;
				
			}

			add_tunnel(p);				
			iph->daddr() = prefix;
			
			bu->eface()->send(p,0);
		
	}
	
	debug(
			"At %f MIPv6 Agent in %s send binding update message using interface %s\n", 
			NOW, MYNUM, Address::instance().print_nodeaddr(iface->address()));

}
void MIPV6Agent::send_cn_bu_msg(Packet* p, int prefix) {
	
}

void MIPV6Agent::send_bu_ack(Packet* p) {
	
	hdr_ip *iph= HDR_IP(p);
//	hdr_cmn *hdrc= HDR_CMN(p);
	hdr_nemo *nh= HDR_NEMO(p);
	
	//reply packet
	Packet *p_ack = p->copy();
	hdr_ip *iph_ack= HDR_IP(p_ack);
	hdr_nemo *nh_ack= HDR_NEMO(p_ack);
	hdr_cmn *hdrc_ack= HDR_CMN(p_ack);
	
	
	nh_ack->type()=BACK;
	nh_ack->lifetime()=0;
	nh_ack->seqno()=nh->seqno();

	iph_ack->saddr() = addr();
	iph_ack->daddr() = nh->coa();
	iph_ack->dport() = port();
	hdrc_ack->ptype() = PT_NEMO;
	hdrc_ack->size() = IPv6_HEADER_SIZE + BACK_SIZE;
	
	if(iph->saddr()!=nh->coa())
	{
		add_tunnel(p_ack);
		iph_ack->daddr()=iph->saddr();
	}
	
	debug("At %f MIPv6 Agent in %s send binding update ack message\n", NOW, MYNUM);

	send(p_ack, 0);
	
}

//void MIPV6Agent::send_bu_ack(Packet* p, Node* iface) {
//	hdr_nemo *nh= HDR_NEMO(p);
//	
//	//reply packet
//	Packet *p_ack = allocpkt();
//	hdr_ip *iph_ack= HDR_IP(p_ack);
//	hdr_nemo *nh_ack= HDR_NEMO(p_ack);
//	hdr_cmn *hdrc_ack= HDR_CMN(p_ack);
//
//	nh_ack->type()=BACK;
//	nh_ack->lifetime()=0;
//	nh_ack->seqno()=nh->seqno();
//
//	iph_ack->saddr() = addr();
//	iph_ack->daddr() = nh->coa();
//	iph_ack->dport() = port();
//	hdrc_ack->ptype() = PT_NEMO;
//	hdrc_ack->size() = IPv6_HEADER_SIZE + BACK_SIZE;
//	
//	
//	debug("At %f MIPv6 Agent in %s send binding update ack message using interface %s \n", 
//			NOW, MYNUM, Address::instance().print_nodeaddr(iface->address()));
//	send(p_ack, 0);
//	Tcl& tcl = Tcl::instance();
//	tcl.evalf("%s entry", iface->name());
//	NsObject* obj = (NsObject*) TclObject::lookup(tcl.result());
//	Scheduler::instance().schedule(obj,p->copy(),0.1);
//}

void MIPV6Agent::recv_bu_ack(Packet* p) {
	//received packet
	hdr_ip *iph= HDR_IP(p);

	assert (HDR_CMN(p)->ptype() == PT_NEMO);

	debug("At %f MIPv6 Agent in %s received binding update ack message from %s\n", 
			NOW, MYNUM, Address::instance().print_nodeaddr(iph->saddr()));
	
}

void MIPV6Agent::tunneling(Packet* p)
{
	hdr_ip *iph= HDR_IP(p);
	hdr_cmn *hdrc= HDR_CMN(p);
	hdr_nemo *nh= HDR_NEMO(p);
	//hdr_mysip* siph = hdr_mysip::access(p);
		
	debug("MIPV6Agent::tunneling\n");
	debug("sizeof packet %d\n",hdrc->size());
	if(node_type_==MR)
	{
		debug("MIPv6 MR\n");
		
		//	delete tunnel and look up daddr 
		//	daddr is in binding table?
		//	Yes -> send to MN caddr (NEMO prefix addr) by iface
		//	add tunnel and change addr to NEMO prefix addr
		//	send by iFace agent 
		//	No find -> send to MR_HA by eface
		//	add tunnel and chage addr to MR_HA
		//	send by eface agent
//		int mr_ha_addr = iph->saddr();
		delete_tunnel(p);
		
//		BUEntry* bu = get_entry_by_haddr(iph->daddr());
//
//		//assert(bu!=NULL);
//		if(!bu)
//		{
//			//	daddr search by haddr is not in binding table 
			BUEntry* bu = get_entry_by_addr(iph->daddr());
			if(!bu)
			{
				//	daddr search by caddr is not in binding table 
				
				bu = get_entry_by_addr(iph->saddr());
				
				//	mn entry -> compare if eface is the same as coa
				//	if different ->  tunnel to the ori ha
				if( bu->caddr()!=bu->eface()->get_iface()->address() ) {
					BUEntry* tunnel = get_mr_ha_entry_by_caddr(bu->eface()->get_iface()->address());
					add_tunnel(p);
					iph->daddr() = tunnel->addr();
					iph->dport()= port();
					debug("MR tunnel-in-tunnel to other MR-HA\n");
				}
				
				BUEntry* out = get_mr_ha_entry_by_caddr(bu->caddr());
				
				iph->saddr()=out->haddr();
//				nh->coa()=out->haddr();
				add_tunnel(p);
				
				iph->daddr() = out->addr();
				iph->dport()= port();
				Packet* p_tunnel = p->copy();
				out->eface()->send(p_tunnel,0);
				
//				int prefix = iph->saddr() & 0xFFFFF800;
//				debug ("prefix=%s \n", Address::instance().print_nodeaddr(prefix));
//				bu = get_entry_by_prefix(prefix);
//				iph->daddr()=bu->addr();
//				iph->dport()=port();
//				bu->eface()->send(p_tunnel,0);

			} else {
				//	daddr search by caddr is in binding table 
				add_tunnel(p);
				int prefix = iph->daddr() & 0xFFFFF800;
				debug ("prefix=%s \n", Address::instance().print_nodeaddr(prefix));
				iph->daddr()=iph->daddr();
				iph->dport()=port();
//				iph->saddr()=mr_ha_addr;
				Packet* p_tunnel = p->copy();
				get_iface_agent_by_prefix(prefix)->send(p_tunnel,0);
			}
//			
//			
//		} else {
//			//	daddr search by haddr is in binding table 
//			//	check saddr is in the binding table (CN)  
//			//	if not -> create a new CN binding entry
//			//	
//			
//			add_tunnel(p);
//			//debug("bu->addr() %s \n",Address::instance().print_nodeaddr(bu->addr()));
//			int prefix = bu->addr() & 0xFFFFF800;
//			debug ("prefix=%s \n", Address::instance().print_nodeaddr(prefix));
//			iph->daddr()=bu->addr();
//			iph->dport()=port();
//			iph->saddr()=mr_ha_addr;
//			Packet* p_tunnel = p->copy();
//			get_iface_agent_by_prefix(prefix)->send(p_tunnel,0);
//
//		}
		
		debug("At %f MIPv6 MR Agent in %s tunnel packet to %s\n", 
				NOW, MYNUM, Address::instance().print_nodeaddr(iph->daddr()));
		
	}else if (node_type_==MN){
		debug("MIPv6 MN\n");
		//	packet saddr is me?
		//	Yes -> add tunnel and send to ap
		//	NO -> delete tunnel and send to agent
		//	if cn not in binding table
		//	register and send BU
		
		BUEntry *bu =  get_entry_by_type(MN_HA);
		
		if(iph->saddr()==bu->eface()->get_iface()->address())
		{
			
			//	lookup if destination in the binding table 
			//	if not then tunnel packet to HA
			add_tunnel(p);
			BUEntry *dest = get_entry_by_caddr(iph->daddr());
			if(!dest)
			{
				
				//	send packet to HA
				//bu = bulist_head_.lh_first;
				//bu = get_entry_by_type(MN_HA);
				
				//	save daddr temporary in nh->haddr
				nh->haddr()=bu->haddr();
				nh->coa()=bu->caddr();
				
				iph->daddr()=bu->addr();
				iph->dport()=port();
				
			} 
			else {
				iph->daddr()=dest->addr();
				iph->dport()=port();
				
			}
			
//			add_tunnel(p);
//			
//			BUEntry* bu = get_entry_by_haddr(iph->daddr());
//			if(!bu)
//			{
//				//	send packet to MN_HA
//				bu =  bulist_head_.lh_first;
//				
//				iph->daddr()=bu->addr();
//				iph->dport()=port();
//				
//				nh->haddr()=bu->haddr();
//				nh->coa()=addr();
//			} else {
//				//	add tunnel and set addr to MN caddr
//				//	add tunnel and send packet to MN_HA
//				iph->daddr()=bu->caddr();
//				iph->dport()=port();
//				add_tunnel(p);
//				iph->daddr()=bu->addr();
//				iph->dport()=port();
//				
//				nh->haddr()=bu->haddr();
//				nh->coa()=addr();
//			}
			
			//	add tunnel and send to ap
			//	change addr to Ap
			int prefix = iph->saddr() & 0xFFFFF800;
			debug ("prefix=%s \n", Address::instance().print_nodeaddr(prefix));
			
			add_tunnel(p);
			iph->daddr()=prefix;
			iph->dport()=port();
			
			hdrc->size()-=20;
			
			Packet* p_tunnel = p->copy();
			bu->eface()->send(p_tunnel,0);
			debug("At %f MIPv6 MN Agent in %s send tunnel packet to %s\n", 
							NOW, MYNUM, Address::instance().print_nodeaddr(iph->daddr()));
			
		} else {
			//	delete tunnel and send to agent
			//	if CN not in binding table
			//	register and send BU to CN
			
			int mr_ha_addr=iph->saddr();
			delete_tunnel(p);
			
			if(size_tunnel(p)>0)
				delete_tunnel(p);
			
			int mn_addr=iph->saddr();

			debug("MN mr_ha_addr %s mn_addr %s\n", 
					Address::instance().print_nodeaddr(mr_ha_addr),
					Address::instance().print_nodeaddr(mn_addr));
			bu = get_entry_by_caddr(mn_addr);
			iph->saddr() = mr_ha_addr;
//			iph->saddr() = mr_ha_addr;
//			bu = get_entry_by_addr(iph->saddr());
			//bu = get_entry_by_haddr(nh->haddr());
			if(!bu)
			{
				registration(p,MN);
				dump();
				bu =  get_entry_by_type(MN_HA);
				int my_addr = bu->eface()->get_iface()->address();
				int prefix = my_addr & 0xFFFFF800;
				debug ("prefix=%s \n", Address::instance().print_nodeaddr(prefix));
				
				Packet* p_bu = allocpkt();
				hdr_ip *bu_iph= HDR_IP(p_bu);
				hdr_cmn *bu_hdrc= HDR_CMN(p_bu);
				hdr_nemo *bu_nh= HDR_NEMO(p_bu);
				bu_nh->coa() = my_addr;
				bu_nh->haddr() = bu->haddr();	
				bu_nh->H()=ON;
				bu_nh->A()=ON;
				bu_nh->type()=BU;
				bu_nh->lifetime()=TIME_INFINITY;
				bu_nh->seqno()=0;
								
				bu_iph->saddr() = my_addr;
				bu_iph->sport() = port();
				bu_iph->daddr()= mn_addr;
				bu_iph->dport() = port();
				bu_hdrc->ptype() = PT_NEMO;
				bu_hdrc->size() = IPv6_HEADER_SIZE + BU_SIZE;
				
				add_tunnel(p_bu);
				bu_iph->daddr() = mr_ha_addr;
				
				add_tunnel(p_bu);				
				bu_iph->daddr() = prefix;
				
				bu->eface()->send(p_bu,0);
				debug("At %f MIPv6 MN Agent in %s send BU packet to %s\n", 
						NOW, MYNUM, Address::instance().print_nodeaddr(bu_iph->daddr()));
			}
			
			debug("At %f MIPv6 MN Agent in %s recv tunnel packet\n", NOW, MYNUM);
			
			
//			hdrc->size()+=20;
			iph->daddr()=addr();
					
			Packet* p_untunnel = p->copy();
			send(p_untunnel,0);
			
		}
					
	}else if (node_type_==MR_HA){
		debug("MIPv6 MR_HA\n");
		
		//	delete tunnel and look up daddr 
		//	daddr is in binding table?
		//	Yes -> send to MR caddr 
		//		search if caddr is another MR-HA
		//		Yes -> send to another MR-HA
		//		NO ->	add tunnel and change addr to MR and port
		//	No ->	is nemo prefix? 
		//	Yes -> send to addr 
		//	NO -> add tunnel and chage addr to addr
		int mr_ha_addr = iph->saddr();
		delete_tunnel(p);
		debug("MR_HA mr_ha_addr %s saddr() %s \n", 
				Address::instance().print_nodeaddr(mr_ha_addr),
				Address::instance().print_nodeaddr(iph->saddr()));
		
//		BUEntry* bu = get_entry_by_haddr(iph->daddr());
//		
//		//assert(bu!=NULL);
//		if(!bu)
//		{
			//	check if daddr is nemo prefix
		debug("MR_HA size %d\n",size_tunnel(p));
			int prefix = iph->daddr() & 0xFFFFF800;
			debug ("prefix=%s \n", Address::instance().print_nodeaddr(prefix));
			BUEntry* bu = get_entry_by_prefix(prefix);
			if(!bu)
			{
				bu = get_entry_by_haddr(nh->haddr());
				if(!bu)
				{
					//	if send to CN
					if(iph->daddr()!=addr())
						add_tunnel(p);
					iph->daddr()=iph->daddr();
					iph->dport()=port();

					iph->saddr()=addr();
				} else {
					//	if from anothe HA to MN
					add_tunnel(p);
					iph->daddr()=bu->caddr();
					iph->dport()=port();
					iph->saddr()=addr();
				}

				Packet* p_tunnel = p->copy();
				send(p_tunnel,0);
			} else {
				debug("find nemo prefix\n");
				
				// if saddr different 
				//	-> mean from MR_HA,	we need to keep it
				//	if saddr same
				//	->	no change
				if(iph->saddr()!= mr_ha_addr)
				{
					add_tunnel(p);
					iph->saddr()= mr_ha_addr;
				}
//				iph->saddr()= mr_ha_addr;
				
				BUEntry* other_ha = get_entry_by_haddr(bu->caddr());
				if(!other_ha)
				{
					add_tunnel(p);
					iph->daddr()=bu->caddr();
					iph->dport()=port();

				} else {
					debug("find other_ha\n");
//					add_tunnel(p);
//					iph->daddr()=other_ha->haddr();
//					iph->dport()=port();
					nh->haddr()=other_ha->haddr();
					add_tunnel(p);
					iph->daddr()=other_ha->caddr();
					iph->dport()=port();
				}
				
				Packet* p_tunnel = p->copy();
				send(p_tunnel,0);
			}
						
//		}else{
//			//add_tunnel(p);
//			BUEntry* other_ha = get_entry_by_haddr(bu->caddr());
//			
//			if(!other_ha)
//			{
//				iph->daddr()=bu->caddr();
//				iph->dport()=port();
//				iph->saddr()=mr_ha_addr;
//				
//			} else {
//				debug("find other_ha\n");
//				//add_tunnel(p);
//				iph->daddr()=other_ha->haddr();
//				iph->dport()=port();
//				add_tunnel(p);
//				iph->daddr()=other_ha->caddr();
//				iph->dport()=port();
//			}
//				
//			
//
//			Packet* p_tunnel = p->copy();
//			send(p_tunnel,0);
//			
//		}
		
		debug("At %f MIPv6 MR_HA Agent in %s tunnel packet to %s\n", 
				NOW, MYNUM, Address::instance().print_nodeaddr(iph->daddr()));
		
		
	}else if (node_type_==MN_HA){
		debug("MIPv6 MN_HA\n");
		
		//	delete tunnel and look up MN caddr (MR hoa)
		//	add tunnel and change daddr to MR hoa and port
		//	add tunnel and change daddr to MR_HA and port
		int mr_ha_addr = iph->saddr();
		int mr_ha_flag = 0;
		delete_tunnel(p);
		
		if(size_tunnel(p)>0)
		{
			delete_tunnel(p);
			mr_ha_flag =1;
		}
			
		BUEntry* bu = get_entry_by_haddr(iph->daddr());
		//int mr_ha_addr;
//		int mr_haddr;
		//int mn_addr;
		
//		if(bu==NULL)
//		{
//			//	if packet from MN 
//			
//			mr_haddr = nh->coa();
//			debug("MN_HA mr_ha_addr %s mr_haddr %s\n", 
//					Address::instance().print_nodeaddr(mr_ha_addr),
//					Address::instance().print_nodeaddr(mr_haddr));
//			delete_tunnel(p);
//			bu = get_entry_by_haddr(iph->daddr());
//			
//			assert(bu!=NULL);
//		}
//		//	else if packet from CN
		
		assert(bu!=NULL);
		iph->daddr()=bu->caddr();
		add_tunnel(p);
		iph->daddr()=bu->addr();
		iph->dport()=port();
		if(mr_ha_flag)
			iph->saddr() = mr_ha_addr;
		
//		if (bu->addr()==bu->caddr())
//		{
//			//	packet to CN
//			// add tunnel and change daddr to CN coa
////			delete_tunnel(p);
//			//mn_addr=iph->saddr();
////			hdrc->size()-=20;
//			iph->daddr()=bu->caddr();
//			
//			nh->coa()=mr_haddr;
//			add_tunnel(p);
//			iph->daddr()=bu->caddr();
//			iph->saddr()=mr_ha_addr;
//			iph->dport()=port();
//						
//		} else {
//			//	packet to MN
//			//	add tunnel and change daddr to MR hoa and port
//			//	add tunnel and change daddr to MR_HA and port
//			
//			add_tunnel(p);
//			iph->daddr()=bu->caddr();
//			iph->saddr()=mr_ha_addr;
//
//			add_tunnel(p);
//			iph->daddr()=bu->addr();
//			iph->dport()=port();
//		}
		
		
		Packet* p_tunnel = p->copy();
		send(p_tunnel,0);
		
		debug("At %f MIPv6 MN_HA Agent in %s tunnel packet to %s\n", 
					NOW, MYNUM, Address::instance().print_nodeaddr(iph->daddr()));
		
	}else if (node_type_==CN){
		debug("MIPv6 CN\n");
		//	packet daddr is me?
		//	Yes -> delete tunnel and send to agent
		//	register if mn not in the binding table 
		//	No -> add tunnel and send to mn
		//	first lookup if mn in binding table
		//	if not then tunnel packet to MN_HA
		
		if(iph->daddr()==addr()) {
			//	register if mn not in the binding table 
			//	delete tunnel and send to agent
			//	register if mn not in the binding table 
			int mr_ha_addr=iph->saddr();
			delete_tunnel(p);
			int saddr=iph->saddr();
			
			debug("mr_ha_addr %s saddr %s\n", 
								Address::instance().print_nodeaddr(mr_ha_addr),
								Address::instance().print_nodeaddr(saddr));
			iph->saddr()=mr_ha_addr;
			BUEntry* bu = get_entry_by_addr(iph->saddr());
			if(!bu)
			{
				registration(p,MN);
				dump();
				bu = get_entry_by_type(MN_HA);
				Packet* p_bu = allocpkt();
				hdr_ip *bu_iph = HDR_IP(p_bu);
				hdr_cmn *bu_hdrc= HDR_CMN(p_bu);
				hdr_nemo *bu_nh= HDR_NEMO(p_bu);
				bu_nh->coa() = addr();
				bu_nh->haddr() = bu->haddr();	
				bu_nh->H()=ON;
				bu_nh->A()=ON;
				bu_nh->type()=BU;
				bu_nh->lifetime()=TIME_INFINITY;
				bu_nh->seqno()=0;
								
				bu_iph->saddr() = addr();
				bu_iph->sport() = port();
				bu_iph->daddr()= saddr;
				bu_iph->dport() = port();
				bu_hdrc->ptype() = PT_NEMO;
				bu_hdrc->size() = IPv6_HEADER_SIZE + BU_SIZE;
				
				add_tunnel(p_bu);				
				bu_iph->daddr() = iph->saddr();
				
				send(p_bu,0);

				debug("At %f MIPv6 CN Agent in %s send BU packet to %s\n", 
						NOW, MYNUM, Address::instance().print_nodeaddr(bu_iph->daddr()));
				
			}
			
			iph->daddr()=addr();
//			hdrc->size()-=20;
			Packet* p_untunnel = p->copy();
			send(p_untunnel,0);
			
		} else {
			//	add tunnel and send to mn
			//	first lookup if mn in binding table
			//	if not then tunnel packet to MN_HA
			
			add_tunnel(p);
//			debug("CN iph->daddr() %s\n", Address::instance().print_nodeaddr(iph->daddr()));
			BUEntry* bu = get_entry_by_caddr(iph->daddr());
			if(!bu)
			{
				//	send packet to MN_HA
				//bu =  bulist_head_.lh_first;
				bu = get_entry_by_type(MN_HA);
				iph->daddr()=bu->addr();
				iph->dport()=port();
				
				nh->haddr()=bu->haddr();
				nh->coa()=addr();
			} else {
				//	add tunnel and set addr to MN caddr
				//	add tunnel and send packet to MN_HA
//				debug("CN test\n");
//				iph->daddr()=bu->caddr();
//				iph->dport()=port();
//				add_tunnel(p);
				iph->daddr()=bu->addr();
				iph->dport()=port();
				
				nh->haddr()=bu->haddr();
				nh->coa()=addr();
			}
			Packet* p_tunnel = p->copy();
			send(p_tunnel,0);
			
			debug("At %f MIPv6 MN_CN Agent in %s tunnel packet to %s\n", 
						NOW, MYNUM, Address::instance().print_nodeaddr(iph->daddr()));
			
		}
		
		
	}
					
					
					
//	if(ha!=0)
//	{
//		hdr_ip* ih = HDR_IP(p);
//		if(ih->daddr()==coa_ || ih->daddr()==addr())
//		{
//			debug("At %f MIPv6 HA Agent in %s recv tunnel packet\n", NOW, MYNUM);
//			
//			delete_tunnel(p);
//			Packet* p_untunnel = p->copy();
////			target_->recv(p_untunnel);
//			if(udpmysip_!=0 )
//				udpmysip_->recv(p_untunnel,0);
//			else
//				debug("Unable to find udpmysip\n");
//		}
//		else
//		{
//			debug("At %f MIPv6 Agent in %s tunnel packet to HA\n", NOW, MYNUM);
//			Packet* p_tunnel = p->copy();
//			add_tunnel(p_tunnel);
//			hdr_ip* ih_tunnel = HDR_IP(p_tunnel);
//			ih_tunnel->daddr() = ha_;
//			ih_tunnel->dport() = port();
//			debug("daddr() %s dport() %d\n",Address::instance().print_nodeaddr(ih_tunnel->daddr()), ih_tunnel->dport());
//			send(p_tunnel, 0);
////			add_tunnel(p);
////			ih->daddr() = ha_;
////			ih->dport() = port();
////			debug("daddr() %s dport() %d\n",Address::instance().print_nodeaddr(ih->daddr()), ih->dport());
////			send(p, 0);
//		}
//	} 
//	else 
//	{
//		//	recv by MIPV6 home agent (ha_==0)
//		
//		
//		hdr_ip* ih = HDR_IP(p);
//		
////		hdr_mysip* siph = hdr_mysip::access(p);
////		hdr_nemo* nh = hdr_mysip::access(p);
//		
//		debug("At %f MIPv6 HA Agent in %s tunnel packet to %s\n", 
//				NOW, MYNUM, Address::instance().print_nodeaddr(ih->daddr()));
//		
//		delete_tunnel(p);
//		
//		BUEntry* n = lookup_coa(Address::instance().get_nodeaddr(ih->daddr()), head(bulist_head_));
//		if (n) 
//		{
//			//	Find care-of-address
//			Packet *p_tunnel = p->copy();
//			hdr_ip* ih_tunnel = hdr_ip::access(p_tunnel);
//			
//			add_tunnel(p_tunnel);
//			
//			ih_tunnel->daddr() = n->caddr;
//			ih_tunnel->dport() = port();
//			
//			debug("%s coa %s find\n", Address::instance().print_nodeaddr(ih->daddr()), 
//					Address::instance().print_nodeaddr(n->caddr));
//			
//			send(p_tunnel, 0);
//		} 
//		else 
//		{
//			n = lookup_hoa(Address::instance().get_nodeaddr(ih->daddr()), head(bulist_head_));
//			if(n)
//			{
//				//	Find home address
//				Packet *p_tunnel = p->copy();
//				hdr_ip* ih_tunnel = hdr_ip::access(p_tunnel);
//				
//				add_tunnel(p_tunnel);
//				
//				ih_tunnel->daddr() = n->caddr;
//				ih_tunnel->dport() = port();
//				
//				debug("%s hoa %s find\n", Address::instance().print_nodeaddr(ih->daddr()),
//						Address::instance().print_nodeaddr(n->haddr));
//				
//				send(p_tunnel, 0);
//			}
//			else
//			{
//				//	Not find care-of-address and home address
//				debug("coa or hoa not find\n");
//				send(p,0);
//				Packet::free(p);
//			}
//		}
//			
//	}
}

void MIPV6Agent::add_tunnel(Packet* p)
{
	hdr_cmn *hdrc= HDR_CMN(p);
	hdr_ip *iph= HDR_IP(p);
	hdr_tunnel *th = HDR_TUNNEL(p);
	
	tunnel *new_tunnel = (tunnel*) malloc(sizeof(tunnel));
	new_tunnel->daddr = iph->daddr();
	new_tunnel->dport = iph->dport();
	new_tunnel->saddr	= iph->saddr();
	new_tunnel->sport	= iph->sport();
	
	hdrc->size() += IPv6_HEADER_SIZE;
//	debug("hdrc->size() %d\n",hdrc->size());
	debug("At %f MIPv6 Agent in %s add_tunnel daddr %s dport %d saddr %s sport %d\n", NOW, MYNUM,
			Address::instance().print_nodeaddr(new_tunnel->daddr), new_tunnel->dport,
			Address::instance().print_nodeaddr(new_tunnel->saddr), new_tunnel->sport);
	
	th->tunnels().push_back(new_tunnel);
	debug("tunnels() size %d\n",th->tunnels().size());
}

void MIPV6Agent::delete_tunnel(Packet* p)
{
	hdr_cmn *hdrc= HDR_CMN(p);
	hdr_ip *iph= HDR_IP(p);
	hdr_tunnel *th = HDR_TUNNEL(p);
	
	assert(th->tunnels().size());
	
	vector <tunnel*>::iterator last_tunnel = th->tunnels().end()-1;
	
	hdrc->size() -= IPv6_HEADER_SIZE;
//	debug("hdrc->size() %d\n",hdrc->size());
	debug("At %f MIPv6 Agent in %s delete_tunnel daddr %s dport %d saddr %s sport %d\n", NOW, MYNUM,
			Address::instance().print_nodeaddr((*last_tunnel)->daddr), (*last_tunnel)->dport,
			Address::instance().print_nodeaddr((*last_tunnel)->saddr), (*last_tunnel)->sport);
	
	iph->daddr() = (*last_tunnel)->daddr;
	iph->dport() = (*last_tunnel)->dport;
	iph->saddr() = (*last_tunnel)->saddr;
	iph->sport() = (*last_tunnel)->sport;
	
	th->tunnels().erase(th->tunnels().end()-1);
	debug("tunnels() size %d\n",th->tunnels().size());
}

int MIPV6Agent::size_tunnel(Packet* p)
{
	hdr_tunnel *th = HDR_TUNNEL(p);
	return th->tunnels().size();
}

/*
 * Copmute the new address using the new prefix received
 * @param prefix The new prefix received
 * @param interface The interface to update
 */
int MIPV6Agent::compute_new_address (int prefix, Node *interface)
{
  int new_address;
  int old_address = interface->address();
  Tcl& tcl = Tcl::instance();

  new_address = (old_address & 0x7FF)|(prefix & 0xFFFFF800);

  char *os = Address::instance().print_nodeaddr(old_address);
  char *ns = Address::instance().print_nodeaddr(new_address);
  char *ps = Address::instance().print_nodeaddr(prefix);
  debug ("\told address: %s, prefix=%s, new address will be %s\n", os, ps, ns);

  //update the new address in the node
  tcl.evalf ("%s addr %s", interface->name(), ns);
  tcl.evalf ("[%s set ragent_] addr %s", interface->name(), ns);
  tcl.evalf ("%s base-station [AddrParams addr2id %s]",interface->name(),ps);  
  //if I update the address, then I also need to update the local route...
  
  delete []os;
  delete []ns;
  delete []ps;

  return new_address;

}

NEMOAgent* MIPV6Agent::get_iface_agent_by_prefix(int prefix)
{
	BUEntry *bu =  bulist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->prefix()==prefix) {
			return bu->iface();
		}
	}
	return NULL;
}

NEMOAgent* MIPV6Agent::get_eface_agent_by_prefix(int prefix)
{
	BUEntry *bu =  bulist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->prefix()==prefix) {
			return bu->eface();
		}
	}
	return NULL;
}

BUEntry* MIPV6Agent::get_entry_by_iface(Node *iface)
{
	BUEntry *bu =  bulist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->eface()!=NULL && bu->eface()->get_iface()==iface) {
			return bu;
		}
	}
	return NULL;
}

//BUEntry* MIPV6Agent::get_entry_without_iface(Node *iface)
//{
//	BUEntry *bu =  bulist_head_.lh_first;
//	for(;bu;bu=bu->next_entry()) {
//		if(bu->eface()!=NULL && bu->eface()->get_iface()!=iface) {
//			return bu;
//		}
//	}
//	return NULL;
//}

BUEntry* MIPV6Agent::get_entry_by_prefix(int prefix)
{
	BUEntry *bu =  bulist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->prefix()==prefix) {
			return bu;
		}
	}
	return NULL;
}

BUEntry* MIPV6Agent::get_entry_by_haddr(int haddr)
{
	BUEntry *bu =  bulist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->haddr()==haddr) {
			return bu;
		}
	}
	return NULL;
}

BUEntry* MIPV6Agent::get_entry_by_caddr(int caddr)
{
	BUEntry *bu =  bulist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->caddr()==caddr) {
			return bu;
		}
	}
	return NULL;
}

BUEntry* MIPV6Agent::get_entry_by_addr(int addr)
{
	BUEntry *bu =  bulist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->addr()==addr) {
			return bu;
		}
	}
	return NULL;
}
BUEntry* MIPV6Agent::get_entry_by_type(Mipv6NodeType type)
{
	BUEntry *bu =  bulist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->type()==type) {
			return bu;
		}
	}
	return NULL;
}

BUEntry* MIPV6Agent::get_mr_ha_entry_by_caddr(int caddr)
{
	BUEntry *bu =  bulist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->type()==MR_HA && bu->caddr()==caddr) {
			return bu;
		}
	}
	return NULL;
}

BUEntry* MIPV6Agent::get_mr_ha_entry_by_iface(Node *iface)
{
	BUEntry *bu =  bulist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->type()==MR_HA && bu->eface()!=NULL && bu->eface()->get_iface()==iface) {
			return bu;
		}
	}
	return NULL;
}

BUEntry* MIPV6Agent::get_mr_ha_entry_without_iface(Node *iface)
{
	BUEntry *bu =  bulist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->type()==MR_HA && bu->eface()!=NULL && bu->eface()->get_iface()!=iface) {
			return bu;
		}
	}
	return NULL;
}

void MIPV6Agent::re_homing(Node *iface)
{
	BUEntry* bu_break = get_mr_ha_entry_by_iface(iface);
	BUEntry* bu_new = get_mr_ha_entry_without_iface(iface);
	BUEntry* bu_mn = get_entry_by_type(MN);
	
	assert(bu_break!=NULL);
	assert(bu_new!=NULL);
	assert(bu_mn!=NULL);
	
	//	set mn coa to new interface
	bu_mn->caddr()=bu_new->caddr();
	
	//	set bu to break mr_ha
	Packet *p = allocpkt();
	hdr_ip *iph = HDR_IP(p);
	hdr_cmn *hdrc = HDR_CMN(p);
	hdr_nemo *nh = HDR_NEMO(p);
	
	debug("mipv6 exp_ %d\n ",exp_);
	
	if(exp_==4)
	{
		
		nh->coa()=bu_new->haddr();
		nh->haddr()=bu_break->haddr();
		nh->H()=ON;
		nh->A()=ON;
		nh->type()=REHOME;

		iph->saddr()=bu_new->addr();
		iph->daddr()=bu_break->addr();
		iph->dport()=port();
		hdrc->ptype()=PT_NEMO;
		hdrc->size()= IPv6_HEADER_SIZE + BU_SIZE;
		
		add_tunnel(p);
		iph->saddr()=bu_new->caddr();
		iph->daddr()=bu_break->addr();
		iph->dport()=port();
		
	} else {
	
	nh->coa()=bu_new->caddr();
	nh->haddr()=bu_break->haddr();
	nh->H()=ON;
	nh->A()=ON;
	nh->type()=BU;
	
	iph->saddr()=bu_new->caddr();
	iph->daddr()=bu_break->addr();
	iph->dport()=port();
	hdrc->ptype()=PT_NEMO;
	hdrc->size()= IPv6_HEADER_SIZE + BU_SIZE;
	
	add_tunnel(p);
	
	}
	
	
	
	bu_new->eface()->send(p,0);
	
	debug("At %f MIPv6 MR  in %s re_homing to %s\n", 
						NOW, MYNUM, Address::instance().print_nodeaddr(iph->daddr()));
	
//	Tcl& tcl = Tcl::instance();
//	tcl.evalf("%s entry", bu_new->eface()->get_iface()->name());
//	NsObject* obj = (NsObject*) TclObject::lookup(tcl.result());
//	Scheduler::instance().schedule(obj,p->copy(),0.1);
	
	
	//Node* iface =  bu_new->eface()->get_iface()->name();
	//{
//	Tcl& tcl = Tcl::instance();
//	tcl.evalf("%s target [%s entry]", this->name(), bu_new->eface()->get_iface()->name());
	//}
}

