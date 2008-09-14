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
MIPV6Agent::MIPV6Agent() : 
	IFMNGMTAgent(), hoa_(0), ha_(0), coa_(0), nemo_prefix_(0), udpmysip_(NULL),	iface_node_(NULL) {
	LIST_INIT(&bulist_head_);
//	LIST_INIT(&tunnel_head_);
//	LIST_INIT(&bslist_head_);
//	LIST_INIT(&history_head_);
	flowRequestTimer_ = new FlowRequestTimer (this);
	seqNumberFlowRequest_ = 0;
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
		if (strcmp(argv[1], "binding")==0)
		{
			iface_node_ = (Node *) TclObject::lookup(argv[2]);
			send_bu_msg(iface_node_);
			return TCL_OK;
		}
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
		if (strcmp(argv[1], "set-nemo-prefix")==0)
		{
			nemo_prefix_ = Address::instance().str2addr(argv[2]);	
			if(nemo_prefix_==0)
				return TCL_ERROR;
			return TCL_OK;
		}
	}
	if (argc==4) 
	{
		if (strcmp(argv[1], "set-ha")==0) 
		{
			ha_ = Address::instance().str2addr(argv[2]);	//home agent address
			hoa_ = Address::instance().str2addr(argv[3]);	//home address
			BUEntry* nn = add_bulist(ha_, BU_HA, ON);
			nn->activate_entry(NOW, TIME_INFINITY);
			dump();
			return TCL_OK;
		}
	}
	if (argc==7)
	{
		if (strcmp(argv[1], "set-ha")==0) 
		{
			int ha_ = Address::instance().str2addr(argv[2]);							//	home agent address
			int hoa_ = Address::instance().str2addr(argv[3]);							//	home address
			int nemo_prefix_ = Address::instance().str2addr(argv[4]);	//	nemo prefix
			Node *eface_ = (Node *)TclObject::lookup(argv[5]);									// using external interface
			Node *iface_ = (Node *)TclObject::lookup(argv[6]);									// using internal interface
			if (eface_==NULL && iface_==NULL)
				return TCL_ERROR;
			BUEntry* bu = new BUEntry(ha_, BU_HA, ON, hoa_, nemo_prefix_, eface_, iface_);
			bu->insert_entry(&bulist_head_);
//			bu->activate_entry(NOW, TIME_INFINITY);
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
	free(data);
}

/*
 * Process an expired prefix
 * @param data The information about the prefix that expired
 */
void MIPV6Agent::process_exp_prefix(exp_prefix* data) {
	//to be defined by subclass
	free(data);
}

BUEntry* MIPV6Agent::add_bulist(int daddr, Mipv6RegType type, int flag) {
	BUEntry* n = lookup_coa(daddr, head(bulist_head_));
	if (n == NULL) {
		n = new BUEntry(daddr, type, flag);
		n->insert_entry(&bulist_head_);
	}
	//n->activate_entry(NOW, reglftm_);
	return n;
}

void MIPV6Agent::dump() {
	dump_list(head(bulist_head_), "Binding Update List");
//	dump_list(head(bslist_head_), "Base Station List");
//	dump_list(head(history_head_), "History List");
}

void MIPV6Agent::dump_list(BUEntry* node, char* txt) {
	if (node) {
		
//		debug("\n|%s for node %s at %d\n",txt,PRINTADDR(addr()),NOW);
//		debug("|Node \t HoA \t CoA \t Type \t Info\t	Mac\n");
//		for (; node; node=node->next_entry() ) {
//			debug("|%s \t %s \t %s \t %d \t %s \t %d \n", PRINTADDR(node->addr), PRINTADDR(node->haddr), PRINTADDR(node->caddr),
//					node->type, node->info, node->mac_->addr());
//		}
		
		cout <<"\n|"<< txt << " for node "<< PRINTADDR(addr()) <<" at "<< NOW <<"\n";
		cout <<"|Node \t HoA \t CoA \t Type \t Info \t NEMO_prefixe \t Face \t iFace \n";

		for (; node; node=node->next_entry() ) {
			if(node->eface()!=NULL && node->iface()!=NULL)
			{
			cout <<"|"<< PRINTADDR(node->addr) <<"\t" << PRINTADDR(node->haddr) <<"\t"<< PRINTADDR(node->caddr) 
			<<"\t"<< node->type <<"\t "<< node->info  <<"\t" << PRINTADDR(node->prefix()) << "\t"
			<< PRINTADDR(node->eface()->address()) <<"\t" << PRINTADDR(node->iface()->address()) <<"\n";
			}else{
				cout <<"|"<< PRINTADDR(node->addr) <<"\t" << PRINTADDR(node->haddr) <<"\t"<< PRINTADDR(node->caddr) 
				<<"\t"<< node->type <<"\t "<< node->info <<"\t" << PRINTADDR(node->prefix()) << "\n";
			}
		}

		cout << "\n";
	}
}

BUEntry* MIPV6Agent::lookup_coa(int addr, BUEntry* n) {
	for (; n; n=n->next_entry() ) 
	{
		if (n->addr == addr) 
		{
			return n;
		}
	}
	return NULL;
}

BUEntry* MIPV6Agent::lookup_hoa(int addr, BUEntry* n) {
	for (; n; n=n->next_entry() ) 
	{
		if (n->caddr == addr) 
		{
			return n;
		}
	}
	return NULL;
}

BUEntry* MIPV6Agent::lookup_entry(int addr, int coa, BUEntry* n) {
	for ( ; n ; n=n->next_entry() )
	{
		if ( n->addr == addr && n->caddr == coa )
		{
			return n;
		}
	}
	return NULL;
}

void MIPV6Agent::recv_nemo(Packet* p) {
		
	debug("At %f MIPv6 Agent in %s received nemo packet\n", NOW, MYNUM);
	
	hdr_ip *iph= HDR_IP(p);
	hdr_nemo* nh= HDR_NEMO(p);
	
	if(node_type_==MR)
	{
		//	recv by MIPV6 mobile router
		debug("MIPv6 MR\n");
		if(nh->type()==BU) 
		{
			if (nh->H()==ON) 
			{
				//	mn has ha, register to my-ha and mn-ha
				delete_tunnel(p);
				mn_registration(p);
				dump();
				//	test sending back by iface
				Node *iface = get_iface_node_by_daddr(iph->daddr());
				if(iface==NULL)
					printf("NULL");
				assert(iface!=NULL);
				//printf("test");
				send_bu_ack(p,iface);
				
			} else {
				//	mn has no ha, register myself
				mn_registration(p);
				if (nh->A()==ON) 
				{
					send_bu_ack(p);
				}
			}
			
		}
		else if (nh->type()==BACK) 
		{
			recv_bu_ack(p);
		}
		
			
		
	} else {
		printf("MIPv6 home agent\n");
		//	recv by MIPV6 home agent (ha_==0)
		if(nh->type()==BU) 
		{
			if (nh->H()==ON) 
			{
				delete_tunnel(p);
				mn_registration(p);
				dump();
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

void MIPV6Agent::mn_registration(Packet* p) {
	hdr_nemo* nh= HDR_NEMO(p);
	
	printf("MIPV6Agent::registration hdr_nemo---> coa %s haddr %s r %d\n",
			Address::instance().print_nodeaddr(nh->coa()), Address::instance().print_nodeaddr(nh->haddr()), nh->R() );
	
	BUEntry* n = lookup_entry(nh->haddr(),nh->coa(),head(bulist_head_));
	if(!n)
	{
		n = new BUEntry(nh->haddr(),BU_MN,OFF);
		n->insert_entry(&bulist_head_);
	}
	n->update_entry(nh->seqno(), nh->haddr(), nh->coa(), NOW, nh->lifetime());
}

//void MIPV6Agent::update_bu_entry(Packet* p, int flag) {
//	printf("mysipapp addr %s\n",PRINTADDR(addr()));
//	printf("mysipapp daddr %s\n",PRINTADDR(daddr()));
//	
//	hdr_nemo* nh= HDR_NEMO(p);
//	printf("MIPV6Agent::update_bu_entry -> coa %s hddr %s H %d A %d\n",
//			Address::instance().print_nodeaddr(nh->coa()),
//			Address::instance().print_nodeaddr(nh->haddr()), nh->H(), nh->A() );
//}

void MIPV6Agent::send_bu_msg() {
	
	Packet *p = allocpkt();
	hdr_ip *iph= HDR_IP(p);
	hdr_cmn *hdrc= HDR_CMN(p);
	
	
	//----------------sem start------------------//
	hdr_nemo *nh= HDR_NEMO(p);
	nh->haddr() = hoa_;
	nh->coa()=addr();
	nh->H()=ON;
	nh->A()=ON;
	nh->type()=BU;
	nh->lifetime()=TIME_INFINITY;
	nh->seqno()=0;
	//----------------sem end------------------//

	iph->saddr() = addr(); //we overwrite to use the interface address
	iph->daddr() = ha_;
	iph->dport() = port();
	hdrc->ptype() = PT_NEMO;
	hdrc->size() = IPv6_HEADER_SIZE + BU_SIZE;
	
	add_tunnel(p);
	debug("At %f MIPv6 Agent in %s send binding update\n", NOW, MYNUM);

	send(p, 0);

}

void MIPV6Agent::send_bu_msg(Node *iface) {
	coa_ = iface->address();
	
	Packet *p = allocpkt();
	hdr_ip *iph= HDR_IP(p);
	hdr_cmn *hdrc= HDR_CMN(p);

	//----------------sem start------------------//
	hdr_nemo *nh= HDR_NEMO(p);
	nh->haddr() = hoa_;
	nh->coa()=iface->address();
	nh->H()=ON;
	nh->A()=ON;
	nh->type()=BU;
	nh->lifetime()=TIME_INFINITY;
	nh->seqno()=0;
	//----------------sem end------------------//
	
	iph->saddr() = iface->address(); //we overwrite to use the interface address	
	iph->dport() = port();
	hdrc->ptype() = PT_NEMO;
	hdrc->size() = IPv6_HEADER_SIZE + BU_SIZE;
	add_tunnel(p);
	//we need to send using the interface
//	{
//		Tcl& tcl = Tcl::instance();
//		tcl.evalf("%s target [%s entry]", this->name(), iface->name());
//	}
	
	Tcl& tcl = Tcl::instance();
	tcl.evalf("%s entry", iface->name());
	NsObject* obj = (NsObject*) TclObject::lookup(tcl.result());
	Scheduler::instance().schedule(obj,p->copy(),0.1);
	
	debug(
			"At %f MIPv6 Agent in %s send binding update message using interface %s\n", 
			NOW, MYNUM, Address::instance().print_nodeaddr(iface->address()));

//	send(p, 0);
	
//	obj-> recv(p);

}

void MIPV6Agent::send_bu_msg(int prefix, Node *iface) {
	coa_ = iface->address();
	
	Packet *p = allocpkt();
	hdr_ip *iph= HDR_IP(p);
	hdr_cmn *hdrc= HDR_CMN(p);

	//----------------sem start------------------//
	hdr_nemo *nh= HDR_NEMO(p);
	
	if(ha_==0)	//MN has no HA
		nh->haddr() = prefix;
	else
		nh->haddr() = hoa_;
	
	nh->coa()=iface->address();
	nh->H()=ON;
	nh->A()=ON;
	nh->type()=BU;
	nh->lifetime()=TIME_INFINITY;
	nh->seqno()=0;
	if(nemo_prefix_!=0)
			nh->nemo_prefix()=nemo_prefix_;
	//----------------sem end------------------//
	
	iph->saddr() = iface->address(); //we overwrite to use the interface address
	if(nemo_prefix_==0)	//	MN node
		iph->daddr() = prefix;
	else	//	MR node
		iph->daddr() = ha_;
			
	iph->dport() = port();
	hdrc->ptype() = PT_NEMO;
	hdrc->size() = IPv6_HEADER_SIZE + BU_SIZE;
	add_tunnel(p);
	//we need to send using the interface
//	{
//		Tcl& tcl = Tcl::instance();
//		tcl.evalf("%s target [%s entry]", this->name(), iface->name());
//	}
	
	Tcl& tcl = Tcl::instance();
	tcl.evalf("%s entry", iface->name());
	NsObject* obj = (NsObject*) TclObject::lookup(tcl.result());
	Scheduler::instance().schedule(obj,p->copy(),0.1);
	
	debug(
			"At %f MIPv6 Agent in %s send binding update message using interface %s\n", 
			NOW, MYNUM, Address::instance().print_nodeaddr(iface->address()));

//	send(p, 0);
	
//	obj-> recv(p);

}
void MIPV6Agent::send_mn_bu_msg(Packet* p, int prefix) {
	
}

void MIPV6Agent::send_bu_ack(Packet* p) {
	
//	hdr_ip *iph= HDR_IP(p);
//	hdr_cmn *hdrc= HDR_CMN(p);
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
	iph_ack->daddr() = nh->coa();
	iph_ack->dport() = port();
	hdrc_ack->ptype() = PT_NEMO;
	hdrc_ack->size() = IPv6_HEADER_SIZE + BACK_SIZE;
	
	
	debug("At %f MIPv6 Agent in %s send binding update ack message\n", NOW, MYNUM);

	send(p_ack, 0);
	
}

void MIPV6Agent::send_bu_ack(Packet* p, Node* iface) {
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
	iph_ack->daddr() = nh->coa();
	iph_ack->dport() = port();
	hdrc_ack->ptype() = PT_NEMO;
	hdrc_ack->size() = IPv6_HEADER_SIZE + BACK_SIZE;
	
	
	debug("At %f MIPv6 Agent in %s send binding update ack message using interface %s \n", 
			NOW, MYNUM, Address::instance().print_nodeaddr(iface->address()));
	send(p_ack, 0);
	Tcl& tcl = Tcl::instance();
	tcl.evalf("%s entry", iface->name());
	NsObject* obj = (NsObject*) TclObject::lookup(tcl.result());
	Scheduler::instance().schedule(obj,p->copy(),0.1);
}

void MIPV6Agent::recv_bu_ack(Packet* p) {
	//received packet
	hdr_ip *iph= HDR_IP(p);

	assert (HDR_CMN(p)->ptype() == PT_NEMO);

	debug("At %f MIPv6 Agent in %s received binding update ack message from %s\n", 
			NOW, MYNUM, Address::instance().print_nodeaddr(iph->saddr()));
	
}

void MIPV6Agent::tunneling(Packet* p)
{
	//printf("MIPV6Agent::tunneling\n");
	if(ha_!=0)
	{
		hdr_ip* ih = HDR_IP(p);
		if(ih->daddr()==coa_ || ih->daddr()==addr())
		{
			debug("At %f MIPv6 HA Agent in %s recv tunnel packet\n", NOW, MYNUM);
			
			delete_tunnel(p);
			Packet* p_untunnel = p->copy();
//			target_->recv(p_untunnel);
			if(udpmysip_!=0 )
				udpmysip_->recv(p_untunnel,0);
			else
				debug("Unable to find udpmysip\n");
		}
		else
		{
			debug("At %f MIPv6 Agent in %s tunnel packet to HA\n", NOW, MYNUM);
			Packet* p_tunnel = p->copy();
			add_tunnel(p_tunnel);
			hdr_ip* ih_tunnel = HDR_IP(p_tunnel);
			ih_tunnel->daddr() = ha_;
			ih_tunnel->dport() = port();
			debug("daddr() %s dport() %d\n",Address::instance().print_nodeaddr(ih_tunnel->daddr()), ih_tunnel->dport());
			send(p_tunnel, 0);
//			add_tunnel(p);
//			ih->daddr() = ha_;
//			ih->dport() = port();
//			debug("daddr() %s dport() %d\n",Address::instance().print_nodeaddr(ih->daddr()), ih->dport());
//			send(p, 0);
		}
	} 
	else 
	{
		//	recv by MIPV6 home agent (ha_==0)
		
		
		hdr_ip* ih = HDR_IP(p);
		
//		hdr_mysip* siph = hdr_mysip::access(p);
//		hdr_nemo* nh = hdr_mysip::access(p);
		
		debug("At %f MIPv6 HA Agent in %s tunnel packet to %s\n", 
				NOW, MYNUM, Address::instance().print_nodeaddr(ih->daddr()));
		
		delete_tunnel(p);
		
		BUEntry* n = lookup_coa(Address::instance().get_nodeaddr(ih->daddr()), head(bulist_head_));
		if (n) 
		{
			//	Find care-of-address
			Packet *p_tunnel = p->copy();
			hdr_ip* ih_tunnel = hdr_ip::access(p_tunnel);
			
			add_tunnel(p_tunnel);
			
			ih_tunnel->daddr() = n->caddr;
			ih_tunnel->dport() = port();
			
			debug("%s coa %s find\n", Address::instance().print_nodeaddr(ih->daddr()), 
					Address::instance().print_nodeaddr(n->caddr));
			
			send(p_tunnel, 0);
		} 
		else 
		{
			n = lookup_hoa(Address::instance().get_nodeaddr(ih->daddr()), head(bulist_head_));
			if(n)
			{
				//	Find home address
				Packet *p_tunnel = p->copy();
				hdr_ip* ih_tunnel = hdr_ip::access(p_tunnel);
				
				add_tunnel(p_tunnel);
				
				ih_tunnel->daddr() = n->caddr;
				ih_tunnel->dport() = port();
				
				debug("%s hoa %s find\n", Address::instance().print_nodeaddr(ih->daddr()),
						Address::instance().print_nodeaddr(n->haddr));
				
				send(p_tunnel, 0);
			}
			else
			{
				//	Not find care-of-address and home address
				debug("coa or hoa not find\n");
				send(p,0);
				Packet::free(p);
			}
		}
			
	}
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

Node* MIPV6Agent::get_iface_node_by_daddr(int daddr)
{
	BUEntry *bu =  bulist_head_.lh_first;
	for(;bu;bu=bu->next_entry()) {
		if(bu->prefix()==daddr) {
			debug("iface %s\n",Address::instance().print_nodeaddr(bu->iface()->address()));
			return bu->iface();
		}
	}
	return NULL;
}
