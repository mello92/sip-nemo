# Test for MutiFaceNodes using Triggers
# @author rouil
# @date 05/19/2005.
# Scenario: Create a multi-interface node using different technologies
#           There is a TCP connection between the router0 and MultiFaceNode.
#           We first use the UMTS interface, then we switch the traffic 
#           to the 802.11 interface when it becomes available. 
#           When the node leaves the coverage area of 802.11, it creates a link going down 
#           event to redirect to UMTS.
#
# Topology scenario:
#
#                                   bstation802(3.0.0)->)
#                                   /
#                                  /      
# router0(1.0.0)---router1(2.0.0)--                              +------------------------------------+
#                                  \                             + iface1:802.11(3.0.1)|              |
#                                   \                            +---------------------+ MutiFaceNode |
#                                   rnc(0.0.0)                   + iface0:UMTS(0.0.2)  |  (4.0.0)     |
#                                      |                         +------------------------------------+
#                                 bstationUMTS(0.0.1)->)         
#                                                                
#                                                   
#                                                                
#              
# 
# 1 Multiface node.

#check input parameters
if {$argc >= 3 || $argc <1} {
	puts ""
	puts "Wrong Number of Arguments! "
	puts "command is ns scenario1.tcl case \[seed\]"
	exit 1
}

global ns

# Define global simulation parameters
Agent/MIHUser/IFMNGMT/MIPV6/Handover/Handover1 set case_ [lindex $argv 0]

# (1,n,n)
Agent/MIHUser/IFMNGMT/MIPV6 set exp_ 4
#Agent/MIHUser/IFMNGMT/MIPV6 set exp_ 0

# seed the default RNG
global defaultRNG
if {$argc == 2} {
	set seed [lindex $argv 1]
	if { $seed == "random"} {
	$defaultRNG seed 0
	} else {
	$defaultRNG seed [lindex $argv 1]
	}
}

#define coverage area for base station: 50m coverage 
Phy/WirelessPhy set Pt_ 0.0134
Phy/WirelessPhy set freq_ 2412e+6
Phy/WirelessPhy set RXThresh_ 5.25089e-10

#define frequency of RA at base station
Agent/ND set maxRtrAdvInterval_ 6
Agent/ND set minRtrAdvInterval_ 2

#define MAC 802_11 parameters
Mac/802_11 set bss_timeout_ 5
Mac/802_11 set pr_limit_ 1.2 ;#for link going down

#
Mac/802_11 set client_lifetime_ 100.0
#

#wireless routing algorithm update frequency (in seconds)
Agent/DSDV set perup_ 8

#define DEBUG parameters
set quiet 0
Agent/ND set debug_ 1 
Agent/MIH set debug_ 1
Agent/MIHUser/IFMNGMT/MIPV6/Handover/Handover1 set debug_ 1
Agent/MIHUser/IFMNGMT/MIPV6	set debug_ 1
Mac/802_11 set debug_ 0
#Mac/802_11 set debug_ 1
Agent/MIHUser/IFMNGMT/MIPV6/Handover/Handover1 set confidence_th_ 100

Agent/NEMO set debug_ 1
Agent/UDP/Udpmysip set debug_ 1

#Rate at which the nodes start moving
set moveStart 10
set moveStop 130
#Speed of the mobile nodes (m/sec)
set moveSpeed 1

#origin of the MN
set X_src 40.0
set Y_src 100.0
set X_dst 160.0
set Y_dst 100.0

#defines function for flushing and closing files
proc finish {} {
	global ns f quiet
	$ns flush-trace
	close $f
	if {$quiet == 0} {
	puts " Simulation ended."
	}
	exit 0
}

# set global variables
set output_dir .

#create the simulator
set ns [new Simulator]
$ns use-newtrace

#open file for trace
set f [open out.res w]
$ns trace-all $f

# set up for hierarchical routing (needed for routing over a basestation)
$ns node-config -addressType hierarchical
AddrParams set domain_num_  20                      ;# domain number
AddrParams set cluster_num_ {1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1}   ;# cluster number for each domain 
# 1st cluster: UMTS: 2 network entities + nb of mobile nodes
# 2nd cluster: CN
# 3rd cluster: core network
# 4th cluster: WLAN: 1BS + nb of mobile nodes
# 5th cluster: super nodes
lappend tmp 4                                      ;# UMTS MNs+RNC+BS
lappend tmp 1                                      ;# cn0
lappend tmp 1                                      ;# router 1
lappend tmp 2                                      ;# bs_eface1_mr0 eface1_mr0
lappend tmp 1                                      ;# mr0
lappend tmp 1                                      ;# ha
lappend tmp 2                                      ;# iface0_mr0 eface_mn0
lappend tmp 1                                      ;# mn0
lappend tmp 1                                      ;# mr0_ha1
lappend tmp 1                                      ;# mn0_ha2
lappend tmp 2                                      ;# bs_eface2_mr0 eface2_mr0

lappend tmp 1                                      ;# mr1_ha1
lappend tmp 1                                      ;# mr1_ha2
lappend tmp 2                                      ;# bs_eface2_mr1 eface2_mr1
lappend tmp 2                                      ;# bs_eface1_mr1 eface1_mr1
lappend tmp 2                                      ;# iface0_mr1 eface0_mn1
lappend tmp 1                                      ;# mn1
lappend tmp 1                                      ;# mr1
lappend tmp 2                                      ;# mface0_mr0 mface0_mr0_lan
lappend tmp 2                                      ;# mface0_mr1 mface0_mr1_lan

AddrParams set nodes_num_ $tmp

array set node_type {MN 0 MN_HA 1 MR 2 MR_HA 3 CN 4}
array set node_type_sip {SIP_MN 0 SIP_MN_HA 1 SIP_MR 2 SIP_MR_HA 3 SIP_CN 4}

# configure UMTS. 
# Note: The UMTS configuration MUST be done first otherwise it does not work
#       furthermore, the node creation in UMTS MUST be as follow
#       rnc, base station, and UE (User Equipment)
$ns set hsdschEnabled_ 1addr
$ns set hsdsch_rlc_set_ 0
$ns set hsdsch_rlc_nif_ 0

# configure RNC node
$ns node-config -UmtsNodeType rnc 
set rnc [$ns create-Umtsnode 0.0.0] ;# node id is 0.
if {$quiet == 0} {
	puts "rnc: tcl=$rnc; id=[$rnc id]; addr=[$rnc node-addr]"
}

# configure UMTS base station
$ns node-config -UmtsNodeType bs \
		-downlinkBW 384kbs \
		-downlinkTTI 10ms \
		-uplinkBW 384kbs \
		-uplinkTTI 10ms \
			 -hs_downlinkTTI 2ms \
			  -hs_downlinkBW 384kbs 

set bsUMTS [$ns create-Umtsnode 0.0.1] ;# node id is 1
if {$quiet == 0} {
	puts "bsUMTS: tcl=$bsUMTS; id=[$bsUMTS id]; addr=[$bsUMTS node-addr]"
}

# connect RNC and base station
$ns setup-Iub $bsUMTS $rnc 622Mbit 622Mbit 15ms 15ms DummyDropTail 2000

$ns node-config -UmtsNodeType ue \
		-baseStation $bsUMTS \
		-radioNetworkController $rnc

set eface0_mr0 [$ns create-Umtsnode 0.0.2] ; #Node Id begins with 2. 
set eface0_mr1 [$ns create-Umtsnode 0.0.3] ;

# Node address for router0 and router1 are 4 and 5, respectively.
set cn0 [$ns node 1.0.0]
set router1 [$ns node 2.0.0]
set ha [$ns node 5.0.0]
set mr0_ha1 [$ns node 8.0.0]
set mr0_ha2 [$ns node 9.0.0]
set mr1_ha1 [$ns node 11.0.0]
set mr1_ha2 [$ns node 12.0.0]

set mface0_mr0 [$ns node 18.0.0]
lappend nodelist $mface0_mr0


set mface0_mr0_lan [$ns newLan $nodelist 10Mb 1ms \
 -llType LL -ifqType Queue/DropTail \
 -macType Mac/802_3 -chanType Channel -address 18.0.1]


set mface0_mr1 [$ns node 19.0.0]
lappend nodelist2 $mface0_mr1

set mface0_mr1_lan [$ns newLan $nodelist2 10Mb 1ms \
 -llType LL -ifqType Queue/DropTail \
 -macType Mac/802_3 -chanType Channel -address 19.0.1]


if {$quiet == 0} {
	puts "cn0: tcl=$cn0; id=[$cn0 id]; addr=[$cn0 node-addr]"
	puts "router1: tcl=$router1; id=[$router1 id]; addr=[$router1 node-addr]"
	puts "ha: tcl=$ha; id=[$ha id]; addr=[$ha node-addr]"
	puts "mr0_ha1: tcl=$mr0_ha1; id=[$mr0_ha1 id]; addr=[$mr0_ha1 node-addr]"
	puts "mr0_ha2: tcl=$mr0_ha2; id=[$mr0_ha2 id]; addr=[$mr0_ha2 node-addr]"
	puts "mr1_ha1: tcl=$mr1_ha1; id=[$mr1_ha1 id]; addr=[$mr1_ha1 node-addr]"
	puts "mr1_ha2: tcl=$mr1_ha2; id=[$mr1_ha2 id]; addr=[$mr1_ha2 node-addr]"
	puts "mface0_mr0: tcl=$mface0_mr0; id=[$mface0_mr0 id]; addr=[$mface0_mr0 node-addr]"
	puts "mface0_mr1: tcl=$mface0_mr1; id=[$mface0_mr1 id]; addr=[$mface0_mr1 node-addr]"
}

# connect links 
$ns duplex-link $rnc $router1 622Mbit 0.4ms DropTail 1000
$ns duplex-link $router1 $cn0 100MBit 30ms DropTail 1000
$ns duplex-link $router1 $ha 100MBit 30ms DropTail 1000
$ns duplex-link $router1 $mr0_ha1 100MBit 30ms DropTail 1000
$ns duplex-link $router1 $mr0_ha2 100MBit 30ms DropTail 1000
$ns duplex-link $router1 $mr1_ha1 100MBit 30ms DropTail 1000
$ns duplex-link $router1 $mr1_ha2 100MBit 30ms DropTail 1000
$ns duplex-link $mface0_mr0 $mface0_mr1 100MBit 30ms DropTail 1000

$rnc add-gateway $router1

# creation of the MutiFaceNodes. It MUST be done before the 802.11
$ns node-config  -multiIf ON                            ;#to create MultiFaceNode 

set mr0 [$ns node 4.0.0] 
set mn0 [$ns node 7.0.0]
set mr1 [$ns node 17.0.0] 
set mn1 [$ns node 16.0.0]

$ns node-config  -multiIf OFF                           ;#reset attribute
if {$quiet == 0} {
	puts "mr0: tcl=$mr0; id=[$mr0 id]; addr=[$mr0 node-addr]"
	puts "mr1: tcl=$mr1; id=[$mr1 id]; addr=[$mr1 node-addr]"
	puts "mn0: tcl=$mn0; id=[$mn0 id]; addr=[$mn0 node-addr]"
	puts "mn1: tcl=$mn1; id=[$mn1 id]; addr=[$mn1 node-addr]"
}

#
# Now we add 802.11 nodes
#

# parameter for wireless nodes
set opt(chan)           Channel/WirelessChannel    ;# channel type for 802.11
set opt(prop)           Propagation/TwoRayGround   ;# radio-propagation model 802.11
set opt(netif)          Phy/WirelessPhy            ;# network interface type 802.11
set opt(mac)            Mac/802_11                 ;# MAC type 802.11
set opt(ifq)            Queue/DropTail/PriQueue    ;# interface queue type 802.11
set opt(ll)             LL                         ;# link layer type 802.11
set opt(ant)            Antenna/OmniAntenna        ;# antenna model 802.11
set opt(ifqlen)         50              	   ;# max packet in ifq 802.11
set opt(adhocRouting)   DSDV                       ;# routing protocol 802.11
set opt(umtsRouting)    ""                         ;# routing for UMTS (to reset node config)

set opt(x)		400			   ;# X dimension of the topography
set opt(y)		400			   ;# Y dimension of the topography

# configure rate for 802.11
Mac/802_11 set basicRate_ 11Mb
Mac/802_11 set dataRate_ 11Mb
Mac/802_11 set bandwidth_ 11Mb

#create the topography
set topo [new Topography]
$topo load_flatgrid $opt(x) $opt(y)
#puts "Topology created"

# create God
create-god 28				                ;# give the number of nodes 


# configure Access Points
$ns node-config  -adhocRouting $opt(adhocRouting) \
				 -llType $opt(ll) \
				 -macType $opt(mac) \
				 -channel [new $opt(chan)] \
				 -ifqType $opt(ifq) \
				 -ifqLen $opt(ifqlen) \
				 -antType $opt(ant) \
				 -propType $opt(prop)    \
				 -phyType $opt(netif) \
				 -topoInstance $topo \
				 -wiredRouting ON \
				 -agentTrace ON \
				 -routerTrace OFF \
				 -macTrace ON  \
				 -movementTrace ON
				 
#######################
#		configure mr 0
# configure Base station 802.11
set bs_eface1_mr0 [$ns node 3.0.0]
#$bs_eface1_mr0 set X_ 100.0
$bs_eface1_mr0 set X_ 50.0
$bs_eface1_mr0 set Y_ 100.0
$bs_eface1_mr0 set Z_ 0.0
if {$quiet == 0} {
	puts "bs_eface1_mr0: tcl=$bs_eface1_mr0; id=[$bs_eface1_mr0 id]; addr=[$bs_eface1_mr0 node-addr]"
}
# we need to set the BSS for the base station
set bs_eface1_mr0_Mac [$bs_eface1_mr0 getMac 0]
set AP_ADDR_0 [$bs_eface1_mr0_Mac id]
if {$quiet == 0} {
	puts "bss_id for bstation 1=$AP_ADDR_0"
}
$bs_eface1_mr0_Mac bss_id $AP_ADDR_0
$bs_eface1_mr0_Mac enable-beacon
$bs_eface1_mr0_Mac set-channel 1


# configure Base station 802.11
set bs_eface2_mr0 [$ns node 10.0.0]
$bs_eface2_mr0 set X_ 110.0
$bs_eface2_mr0 set Y_ 100.0
$bs_eface2_mr0 set Z_ 0.0
if {$quiet == 0} {
	puts "bs_eface2_mr0: tcl=$bs_eface2_mr0; id=[$bs_eface2_mr0 id]; addr=[$bs_eface2_mr0 node-addr]"
}
# we need to set the BSS for the base station
set bs_eface2_mr0_Mac [$bs_eface2_mr0 getMac 0]
set AP_ADDR_0_2 [$bs_eface2_mr0_Mac id]
if {$quiet == 0} {
	puts "bss_id for bstation 2=$AP_ADDR_0_2"
}
$bs_eface2_mr0_Mac bss_id $AP_ADDR_0_2
$bs_eface2_mr0_Mac enable-beacon
$bs_eface2_mr0_Mac set-channel 3

#######################
#		configure mr 1
# configure Base station 802.11
set bs_eface1_mr1 [$ns node 14.0.0]
#$bs_eface1_mr1 set X_ 100.0
$bs_eface1_mr1 set X_ 50.0
$bs_eface1_mr1 set Y_ 100.0
$bs_eface1_mr1 set Z_ 0.0
if {$quiet == 0} {
	puts "bs_eface1_mr1: tcl=$bs_eface1_mr1; id=[$bs_eface1_mr1 id]; addr=[$bs_eface1_mr1 node-addr]"
}
# we need to set the BSS for the base station
set bs_eface1_mr1_Mac [$bs_eface1_mr1 getMac 0]
set AP_ADDR_0 [$bs_eface1_mr1_Mac id]
if {$quiet == 0} {
	puts "bss_id for bs_eface1_mr1 =$AP_ADDR_0"
}
$bs_eface1_mr1_Mac bss_id $AP_ADDR_0
$bs_eface1_mr1_Mac enable-beacon
$bs_eface1_mr1_Mac set-channel 4


# configure Base station 802.11
set bs_eface2_mr1 [$ns node 13.0.0]
$bs_eface2_mr1 set X_ 110.0
$bs_eface2_mr1 set Y_ 100.0
$bs_eface2_mr1 set Z_ 0.0
if {$quiet == 0} {
	puts "bs_eface2_mr1: tcl=$bs_eface2_mr1; id=[$bs_eface2_mr1 id]; addr=[$bs_eface2_mr1 node-addr]"
}
# we need to set the BSS for the base station
set bs_eface2_mr1_Mac [$bs_eface2_mr1 getMac 0]
set AP_ADDR_0_2 [$bs_eface2_mr1_Mac id]
if {$quiet == 0} {
	puts "bss_id for bstation 2=$AP_ADDR_0_2"
}
$bs_eface2_mr1_Mac bss_id $AP_ADDR_0_2
$bs_eface2_mr1_Mac enable-beacon
$bs_eface2_mr1_Mac set-channel 6


# creation of the wireless interface 802.11
$ns node-config -wiredRouting OFF \
				-macTrace ON 		
				
#######################
#		configure mr 0
set eface1_mr0 [$ns node 3.0.1]     ;# node id is 8. 
$eface1_mr0 random-motion 0		;# disable random motion
$eface1_mr0 base-station [AddrParams addr2id [$bs_eface1_mr0 node-addr]] ;#attach mn to basestation
$eface1_mr0 set X_ $X_src
$eface1_mr0 set Y_ $Y_src
$eface1_mr0 set Z_ 0.0

if {$quiet == 0} {
	puts "eface1_mr0 = $eface1_mr0"
}
[$eface1_mr0 getMac 0] set-channel 1


set eface2_mr0 [$ns node 10.0.1]     ;# node id is 8. 
$eface2_mr0 random-motion 0		;# disable random motion
$eface2_mr0 base-station [AddrParams addr2id [$bs_eface2_mr0 node-addr]] ;#attach mn to basestation
$eface2_mr0 set X_ $X_src
$eface2_mr0 set Y_ $Y_src
$eface2_mr0 set Z_ 0.0

if {$quiet == 0} {
	puts "eface2_mr0 = $eface2_mr0"
}
[$eface2_mr0 getMac 0] set-channel 3


set iface0_mr0 [$ns node 6.0.0]     ;# node id is 8. 
$iface0_mr0 random-motion 0		;# disable random motion
#$iface0_mr0 base-station [AddrParams addr2id [$nemo node-addr]] ;#attach mn to basestation
$iface0_mr0 set X_ 200
$iface0_mr0 set Y_ 200
$iface0_mr0 set Z_ 0.0

if {$quiet == 0} {
	puts "iface0_mr0 = $iface0_mr0"
}
#[$iface0_mr0 getMac 0] set-channel 2
set iface0_mr0_Mac [$iface0_mr0 getMac 0]
set AP_ADDR_1 [$iface0_mr0_Mac id]
if {$quiet == 0} {
	puts "bss_id for iface0_mr0 =$AP_ADDR_1"
}
$iface0_mr0_Mac bss_id $AP_ADDR_1
$iface0_mr0_Mac enable-beacon
$iface0_mr0_Mac set-channel 2

set eface0_mn0 [$ns node 6.0.1]     ;# node id is 8. 
$eface0_mn0 random-motion 0		;# disable random motion
$eface0_mn0 base-station [AddrParams addr2id [$iface0_mr0 node-addr]] ;#attach mn to basestation
$eface0_mn0 set X_ 200
$eface0_mn0 set Y_ 170
$eface0_mn0 set Z_ 0.0

if {$quiet == 0} {
	puts "eface0_mn0 = $eface0_mn0"
}
[$eface0_mn0 getMac 0] set-channel 2


#######################
#		configure mr 1
set eface1_mr1 [$ns node 14.0.1]     ;# node id is 8. 
$eface1_mr1 random-motion 0		;# disable random motion
$eface1_mr1 base-station [AddrParams addr2id [$bs_eface1_mr1 node-addr]] ;#attach mn to basestation
$eface1_mr1 set X_ $X_src
$eface1_mr1 set Y_ $Y_src
$eface1_mr1 set Z_ 0.0

if {$quiet == 0} {
	puts "eface1_mr1 = $eface1_mr1"
}
[$eface1_mr1 getMac 0] set-channel 4


set eface2_mr1 [$ns node 13.0.1]     ;# node id is 8. 
$eface2_mr1 random-motion 0		;# disable random motion
$eface2_mr1 base-station [AddrParams addr2id [$bs_eface2_mr1 node-addr]] ;#attach mn to basestation
$eface2_mr1 set X_ $X_src
$eface2_mr1 set Y_ $Y_src
$eface2_mr1 set Z_ 0.0

if {$quiet == 0} {
	puts "eface2_mr1 = $eface2_mr1"
}
[$eface2_mr1 getMac 0] set-channel 6


set iface0_mr1 [$ns node 15.0.0]     ;# node id is 8. 
$iface0_mr1 random-motion 0		;# disable random motion
#$iface0_mr1 base-station [AddrParams addr2id [$nemo node-addr]] ;#attach mn to basestation
$iface0_mr1 set X_ 200
$iface0_mr1 set Y_ 200
$iface0_mr1 set Z_ 0.0

if {$quiet == 0} {
	puts "iface0_mr1 = $iface0_mr1"
}
#[$iface0_mr0 getMac 0] set-channel 2
set iface0_mr1_Mac [$iface0_mr1 getMac 0]
set AP_ADDR_1 [$iface0_mr1_Mac id]
if {$quiet == 0} {
	puts "bss_id for iface0_mr0 =$AP_ADDR_1"
}
$iface0_mr1_Mac bss_id $AP_ADDR_1
$iface0_mr1_Mac enable-beacon
$iface0_mr1_Mac set-channel 5

set eface0_mn1 [$ns node 15.0.1]     ;# node id is 8. 
$eface0_mn1 random-motion 0		;# disable random motion
$eface0_mn1 base-station [AddrParams addr2id [$iface0_mr1 node-addr]] ;#attach mn to basestation
$eface0_mn1 set X_ 200
$eface0_mn1 set Y_ 170
$eface0_mn1 set Z_ 0.0

if {$quiet == 0} {
	puts "eface0_mn1 = $eface0_mn1"
}
[$eface0_mn1 getMac 0] set-channel 5

#
#	calculate the speed of the node
#
#######################
#		configure mr 0
$ns at $moveStart "$eface1_mr0 setdest $X_dst $Y_dst $moveSpeed"
$ns at $moveStart "$eface2_mr0 setdest $X_dst $Y_dst $moveSpeed"
#######################
#		configure mr 1
$ns at $moveStart "$eface1_mr1 setdest $X_dst $Y_dst $moveSpeed"
$ns at $moveStart "$eface2_mr1 setdest $X_dst $Y_dst $moveSpeed"

#
#	add the interfaces to supernode
#
#######################
#		configure mr 0
$mr0 add-interface-node $eface0_mr0
$mr0 add-interface-node $eface1_mr0
$mr0 add-interface-node $eface2_mr0
$mr0 add-interface-node $iface0_mr0
$mn0 add-interface-node $eface0_mn0
#######################
#		configure mr 1
$mr1 add-interface-node $eface0_mr1
$mr1 add-interface-node $eface1_mr1
$mr1 add-interface-node $eface2_mr1
$mr1 add-interface-node $iface0_mr1
$mn1 add-interface-node $eface0_mn1

#
# add link to backbone
#
#######################
#		configure mr 0
$ns duplex-link $bs_eface1_mr0 $router1 100MBit 15ms DropTail 1000
$ns duplex-link $bs_eface2_mr0 $router1 100MBit 15ms DropTail 1000
#######################
#		configure mr 0
$ns duplex-link $bs_eface1_mr1 $router1 100MBit 15ms DropTail 1000
$ns duplex-link $bs_eface2_mr1 $router1 100MBit 15ms DropTail 1000

#
# install ND modules
#
# take care of UMTS
# Note: The ND module is on the rnc node NOT in the base station
set nd_rncUMTS [$rnc install-nd]
$nd_rncUMTS set-router TRUE
$nd_rncUMTS router-lifetime 5
$nd_rncUMTS enable-broadcast FALSE

$nd_rncUMTS add-ra-target 0.0.2 ;#in UMTS there is no notion of broadcast. 
#We fake it by sending unicast to a list of nodes
#######################
#		configure mr 0
set nd_eface0_mr0 [$eface0_mr0 install-nd]
#######################
#		configure mr 1
set nd_eface0_mr1 [$eface0_mr1 install-nd]

# BS WLAN 1
#######################
#		configure mr 0
set nd_bs_eface1_mr0 [$bs_eface1_mr0 install-nd]
$nd_bs_eface1_mr0 set-router TRUE
$nd_bs_eface1_mr0 router-lifetime 18
$ns at 1 "$nd_bs_eface1_mr0 start-ra"

set mipv6_bs_eface1_mr0 [$bs_eface1_mr0 install-default-ifmanager]
set mih_bs_eface1_mr0 [$bs_eface1_mr0 install-mih]
$mipv6_bs_eface1_mr0 connect-mih $mih_bs_eface1_mr0
set tmp2 [$bs_eface1_mr0 set mac_(0)] ;#in 802.11 one interface is created
$tmp2 mih $mih_bs_eface1_mr0
$mih_bs_eface1_mr0 add-mac $tmp2

#######################
#		configure mr 1
set nd_bs_eface1_mr1 [$bs_eface1_mr1 install-nd]
$nd_bs_eface1_mr1 set-router TRUE
$nd_bs_eface1_mr1 router-lifetime 18
$ns at 1 "$nd_bs_eface1_mr1 start-ra"

set mipv6_bs_eface1_mr1 [$bs_eface1_mr1 install-default-ifmanager]
set mih_bs_eface1_mr1 [$bs_eface1_mr1 install-mih]
$mipv6_bs_eface1_mr1 connect-mih $mih_bs_eface1_mr1
set tmp2 [$bs_eface1_mr1 set mac_(0)] ;#in 802.11 one interface is created
$tmp2 mih $mih_bs_eface1_mr1
$mih_bs_eface1_mr1 add-mac $tmp2

# BS WLAN 2
#######################
#		configure mr 0
set nd_bs_eface2_mr0 [$bs_eface2_mr0 install-nd]
$nd_bs_eface2_mr0 set-router TRUE
$nd_bs_eface2_mr0 router-lifetime 18
$ns at 1 "$nd_bs_eface2_mr0 start-ra"

set mipv6_bs_eface2_mr0 [$bs_eface2_mr0 install-default-ifmanager]
set mih_bs_eface2_mr0 [$bs_eface2_mr0 install-mih]
$mipv6_bs_eface2_mr0 connect-mih $mih_bs_eface2_mr0
set tmp3 [$bs_eface2_mr0 set mac_(0)] ;#in 802.11 one interface is created
$tmp3 mih $mih_bs_eface2_mr0
$mih_bs_eface2_mr0 add-mac $tmp3

#######################
#		configure mr 1
set nd_bs_eface2_mr1 [$bs_eface2_mr1 install-nd]
$nd_bs_eface2_mr1 set-router TRUE
$nd_bs_eface2_mr1 router-lifetime 18
$ns at 1 "$nd_bs_eface2_mr1 start-ra"

set mipv6_bs_eface2_mr1 [$bs_eface2_mr1 install-default-ifmanager]
set mih_bs_eface2_mr1 [$bs_eface2_mr1 install-mih]
$mipv6_bs_eface2_mr1 connect-mih $mih_bs_eface2_mr1
set tmp3 [$bs_eface2_mr1 set mac_(0)] ;#in 802.11 one interface is created
$tmp3 mih $mih_bs_eface2_mr1
$mih_bs_eface2_mr1 add-mac $tmp3


# MR 
#######################
#		configure mr 0
set nd_eface1_mr0 [$eface1_mr0 install-nd]
set nd_eface2_mr0 [$eface2_mr0 install-nd]

set nd_iface0_mr0 [$iface0_mr0 install-nd]
$nd_iface0_mr0 set-router TRUE
$nd_iface0_mr0 router-lifetime 18
$ns at 1 "$nd_iface0_mr0 start-ra"

#######################
#		configure mr 1
set nd_eface1_mr1 [$eface1_mr1 install-nd]
set nd_eface2_mr1 [$eface2_mr1 install-nd]

set nd_iface0_mr1 [$iface0_mr1 install-nd]
$nd_iface0_mr1 set-router TRUE
$nd_iface0_mr1 router-lifetime 18
$ns at 1 "$nd_iface0_mr1 start-ra"

# MN
#######################
#		configure mr 0
set nd_eface0_mn0 [$eface0_mn0 install-nd]

#######################
#		configure mr 1
set nd_eface0_mn1 [$eface0_mn1 install-nd]

#
# install NEMO modules
#
#######################
#		configure mr 0
set nemo_eface0_mr0 [$mr0 install-nemo 200]
set nemo_eface1_mr0 [$mr0 install-nemo 201]
set nemo_eface2_mr0 [$mr0 install-nemo 202]
set nemo_iface0_mr0 [$mr0 install-nemo 203]
set nemo_eface0_mn0 [$mn0 install-nemo 200]

$nemo_eface0_mr0 connect-interface $eface0_mr0
$nemo_eface1_mr0 connect-interface $eface1_mr0
$nemo_eface2_mr0 connect-interface $eface2_mr0
$nemo_iface0_mr0 connect-interface $iface0_mr0
$nemo_eface0_mn0 connect-interface $eface0_mn0

#######################
#		configure mr 1
set nemo_eface0_mr1 [$mr1 install-nemo 200]
set nemo_eface1_mr1 [$mr1 install-nemo 201]
set nemo_eface2_mr1 [$mr1 install-nemo 202]
set nemo_iface0_mr1 [$mr1 install-nemo 203]
set nemo_eface0_mn1 [$mn1 install-nemo 200]

$nemo_eface0_mr1 connect-interface $eface0_mr1
$nemo_eface1_mr1 connect-interface $eface1_mr1
$nemo_eface2_mr1 connect-interface $eface2_mr1
$nemo_iface0_mr1 connect-interface $iface0_mr1
$nemo_eface0_mn1 connect-interface $eface0_mn1

#
# install Interface Management modules (MIPv6 or Handover)
#

# MN
#######################
#		configure mr 0
set mipv6_mn0 [$mn0 install-default-ifmanager]
$nd_eface0_mn0 set-ifmanager $mipv6_mn0
$mipv6_mn0 set-node-type $node_type(MN)
$mipv6_mn0 set-mn 5.0.0 5.0.1 $nemo_eface0_mn0

#######################
#		configure mr 1
set mipv6_mn1 [$mn1 install-default-ifmanager]
$nd_eface0_mn1 set-ifmanager $mipv6_mn1
$mipv6_mn1 set-node-type $node_type(MN)
$mipv6_mn1 set-mn 5.0.0 5.0.2 $nemo_eface0_mn1


# MR
#######################
#		configure mr 0
set handover_mr0 [new Agent/MIHUser/IFMNGMT/MIPV6/Handover/Handover1]
$mr0 install-ifmanager $handover_mr0

$nd_eface1_mr0 set-ifmanager $handover_mr0 
$nd_eface2_mr0 set-ifmanager $handover_mr0 
$nd_eface0_mr0 set-ifmanager $handover_mr0 

$handover_mr0 set-node-type $node_type(MR)

#(1,1,n)
#$handover_mr0 set-mr 8.0.0 8.0.1 6.0.0 $nemo_eface1_mr0 $nemo_iface0_mr0
#$handover_mr0 set-mr 8.0.0 8.0.2 100.0.0 $nemo_eface2_mr0 $nemo_iface0_mr0

#(1,n,1)
#$handover_mr0 set-mr 8.0.0 8.0.1 6.0.0 $nemo_eface1_mr0 $nemo_iface0_mr0
#$handover_mr0 set-mr 9.0.0 8.0.2 100.0.0 $nemo_eface2_mr0 $nemo_iface0_mr0

#(1,n,n)
#$handover_mr0 set-mr 8.0.0 8.0.1 6.0.0 $nemo_eface1_mr0 $nemo_iface0_mr0
#$handover_mr0 set-mr 9.0.0 8.0.2 100.0.0 $nemo_eface2_mr0 $nemo_iface0_mr0

#######################
#		configure mr 1
set handover_mr1 [new Agent/MIHUser/IFMNGMT/MIPV6/Handover/Handover1]
$mr1 install-ifmanager $handover_mr1

$nd_eface1_mr1 set-ifmanager $handover_mr1 
$nd_eface2_mr1 set-ifmanager $handover_mr1 
$nd_eface0_mr1 set-ifmanager $handover_mr1 

$handover_mr1 set-node-type $node_type(MR)

#(1,1,n)
#$handover_mr1 set-mr 11.0.0 11.0.1 15.0.0 $nemo_eface1_mr1 $nemo_iface0_mr1
#$handover_mr1 set-mr 11.0.0 11.0.2 101.0.0 $nemo_eface2_mr1 $nemo_iface0_mr1

#(1,n,1)
#$handover_mr1 set-mr 11.0.0 11.0.1 15.0.0 $nemo_eface1_mr1 $nemo_iface0_mr1
#$handover_mr1 set-mr 12.0.0 11.0.2 101.0.0 $nemo_eface2_mr1 $nemo_iface0_mr1

#(1,n,n)
#$handover_mr1 set-mr 11.0.0 11.0.1 15.0.0 $nemo_eface1_mr1 $nemo_iface0_mr1
#$handover_mr1 set-mr 12.0.0 11.0.2 101.0.0 $nemo_eface2_mr1 $nemo_iface0_mr1


# MR HA
#######################
#		configure mr 0
set mipv6_mr0_ha1	[$mr0_ha1 install-default-ifmanager]
set mipv6_mr0_ha2		[$mr0_ha2 install-default-ifmanager]
$mipv6_mr0_ha1 set-node-type $node_type(MR_HA)
$mipv6_mr0_ha2 set-node-type $node_type(MR_HA)

#######################
#		configure mr 1
set mipv6_mr1_ha1	[$mr1_ha1 install-default-ifmanager]
set mipv6_mr1_ha2		[$mr1_ha2 install-default-ifmanager]
$mipv6_mr1_ha1 set-node-type $node_type(MR_HA)
$mipv6_mr1_ha2 set-node-type $node_type(MR_HA)

#	HA
#######################
set mipv6_ha	[$ha install-default-ifmanager]
$mipv6_ha set-node-type $node_type(MN_HA)

#	CN
#######################
set mipv6_cn0 [$cn0 install-default-ifmanager]
$mipv6_cn0 set-cn 5.0.0 5.0.3
$mipv6_cn0 set-node-type $node_type(CN)


#
# install MIH modules
#

#######################
#		configure mr 0
set mih_mr0 [$mr0 install-mih]	;# install MIH in multi-interface node
$handover_mr0 connect-mih $mih_mr0 ;#create connection between MIH and iface management
$handover_mr0 nd_mac $nd_eface1_mr0 [$eface1_mr0 set mac_(0)]
$handover_mr0 nd_mac $nd_eface2_mr0 [$eface2_mr0 set mac_(0)]

#######################
#		configure mr 1
set mih_mr1 [$mr1 install-mih]	;# install MIH in multi-interface node
$handover_mr1 connect-mih $mih_mr1 ;#create connection between MIH and iface management
$handover_mr1 nd_mac $nd_eface1_mr1 [$eface1_mr1 set mac_(0)]
$handover_mr1 nd_mac $nd_eface2_mr1 [$eface2_mr1 set mac_(0)]


#
#	create traffic: TCP application between router0 and Multi interface node
#
#Create a UDP agent and attach it to node n0
set udp_ [new Agent/UDP]
$udp_ set packetSize_ 1500

if {$quiet == 0} {
	puts "udp on node : $udp_"
}

# Create a CBR traffic source and attach it to udp0
set cbr_ [new Application/Traffic/CBR]
$cbr_ set packetSize_ 500
$cbr_ set interval_ 0.2
$cbr_ attach-agent $udp_

#create an sink into the sink node

# Create the Null agent to sink traffic
set null_ [new Agent/Null] 
	
#Router0 is receiver    
#$ns attach-agent $router0 $null_

#Router0 is transmitter    
$ns attach-agent $cn0 $udp_
	
# Attach the 2 agents
#$ns connect $udp_ $null_ OLD COMMAND
	
#Multiface node is transmitter
#$multiFaceNode attach-agent $udp_ $iface0
#$ifmgmt add-flow $udp_ $null_ $iface0 1 2000.
	
#Multiface node is receiver
$mr0 attach-agent $null_ $eface0_mr0 4
$handover_mr0 add-flow $null_ $udp_ $eface0_mr0 1 ;#2000.
		
$mr1 attach-agent $null_ $eface0_mr1 4
$handover_mr1 add-flow $null_ $udp_ $eface0_mr1 1 ;#2000.
		
#
# set for UMTS
#
# do registration in UMTS. This will create the MACs in UE and base stations
$ns node-config -llType UMTS/RLC/AM \
	-downlinkBW 384kbs \
	-uplinkBW 384kbs \
	-downlinkTTI 20ms \
	-uplinkTTI 20ms \
	-hs_downlinkTTI 2ms \
	-hs_downlinkBW 384kbs

# for the first HS-DCH, we must create. If any other, then use attach-dch
#set dch0($n_) [$ns create-dch $iface0($n_) $udp_($n_)]; # multiface node transmitter
set dch0 [$ns create-dch $eface0_mr0 $null_]; # multiface node receiver
$ns attach-dch $eface0_mr0 $handover_mr0 $dch0
$ns attach-dch $eface0_mr0 $nd_eface0_mr0 $dch0

set dch1 [$ns create-dch $eface0_mr1 $null_]; # multiface node receiver
$ns attach-dch $eface0_mr1 $handover_mr1 $dch1
$ns attach-dch $eface0_mr1 $nd_eface0_mr1 $dch1

#
# Now we can register the MIH module with all the MACs
#
#######################
#		configure mr 0
set tmp2 [$eface0_mr0 set mac_(2)] ;#in UMTS and using DCH the MAC to use is 2 (0 and 1 are for RACH and FACH)
$tmp2 mih $mih_mr0
$mih_mr0 add-mac $tmp2
set tmp2 [$eface1_mr0 set mac_(0)] ;#in 802.11 one interface is created
$tmp2 mih $mih_mr0
$mih_mr0 add-mac $tmp2
set tmp2 [$eface2_mr0 set mac_(0)] ;#in 802.11 one interface is created
$tmp2 mih $mih_mr0
$mih_mr0 add-mac $tmp2

#######################
#		configure mr 1
# Now we can register the MIH module with all the MACs
set tmp2 [$eface0_mr1 set mac_(2)] ;#in UMTS and using DCH the MAC to use is 2 (0 and 1 are for RACH and FACH)
$tmp2 mih $mih_mr1
$mih_mr1 add-mac $tmp2
set tmp2 [$eface1_mr1 set mac_(0)] ;#in 802.11 one interface is created
$tmp2 mih $mih_mr1
$mih_mr1 add-mac $tmp2
set tmp2 [$eface2_mr1 set mac_(0)] ;#in 802.11 one interface is created
$tmp2 mih $mih_mr1
$mih_mr1 add-mac $tmp2


#
#create traffic: mysip application between udp_s and udp_r
#

#	Setup a MM UDP connection
set udp_s [new Agent/UDP/Udpmysip]
set udp_r [new Agent/UDP/Udpmysip]
set udp_server [new Agent/UDP/Udpmysip]

#
#	enable udpmysip	(create udpmysip)
#
#######################
#		configure mr 0
set udp_mr0 [new Agent/UDP/Udpmysip]
set udp_mr0_ha1 [new Agent/UDP/Udpmysip]
set udp_mr0_ha2 [new Agent/UDP/Udpmysip]
#######################
#		configure mr 1
set udp_mr1 [new Agent/UDP/Udpmysip]
set udp_mr1_ha1 [new Agent/UDP/Udpmysip]
set udp_mr1_ha2 [new Agent/UDP/Udpmysip]

set udp_mn0 [new Agent/UDP/Udpmysip]

#
#	enable udpmysip (set-node-type)
#
#######################
#		configure mr 0
$udp_mr0 set-node-type $node_type_sip(SIP_MR)
$udp_mr0_ha1 set-node-type $node_type_sip(SIP_MR_HA)
$udp_mr0_ha2 set-node-type $node_type_sip(SIP_MR_HA)
#######################
#		configure mr 1
$udp_mr1 set-node-type $node_type_sip(SIP_MR)
$udp_mr1_ha1 set-node-type $node_type_sip(SIP_MR_HA)
$udp_mr1_ha2 set-node-type $node_type_sip(SIP_MR_HA)
#######################
#		configure others
$udp_s set-node-type $node_type_sip(SIP_CN)
$udp_r set-node-type $node_type_sip(SIP_MN)
$udp_server set-node-type $node_type_sip(SIP_MN_HA)

$udp_mn0 set-node-type $node_type_sip(SIP_MN)


$udp_r set-sip-mn 88 5.0.0 8888 5.0.0 $nemo_eface0_mn1
$udp_s set-sip-cn 88 5.0.0 1000 5.0.0
#######################
#		configure mr 0
$udp_mr0 set-sip-mr 88 8.0.0 9999 8.0.0 6.0.0 $nemo_eface1_mr0 $nemo_iface0_mr0
$udp_mr0 set-sip-mr 88 9.0.0 9998 9.0.0 100.0.0 $nemo_eface2_mr0 $nemo_iface0_mr0
#######################
#		configure mr 1
$udp_mr1 set-sip-mr 88 11.0.0 9999 11.0.0 15.0.0 $nemo_eface1_mr1 $nemo_iface0_mr1
$udp_mr1 set-sip-mr 88 12.0.0 9998 12.0.0 101.0.0 $nemo_eface2_mr1 $nemo_iface0_mr1

$udp_mn0 set-sip-mn 88 5.0.0 8889 5.0.0 $nemo_eface0_mn0


$cn0 attach $udp_s 3
$ha attach $udp_server 3

$mr0_ha1 attach $udp_mr0_ha1 3
$mr0_ha2 attach $udp_mr0_ha2 3
$mr1_ha1 attach $udp_mr1_ha1 3
$mr1_ha2 attach $udp_mr1_ha2 3

$mipv6_cn0 set-udpmysip $udp_s
$mipv6_mn1 set-udpmysip $udp_r
$handover_mr0 set-udpmysip $udp_mr0
$handover_mr1 set-udpmysip $udp_mr1

$mipv6_mn0 set-udpmysip $udp_mn0

#$udp_s set-mipv6 $mipv6_cn0
#$udp_r set-mipv6 $mipv6_mn1

$mn0 attach-agent $udp_mn0 $eface0_mn0 3

$mr0 attach-agent $udp_mr0	$eface0_mr0 3
$mr1 attach-agent $udp_mr1	$eface0_mr1 3

#$mn0 attach-agent $udp_r $eface0_mn0 3
#$handover_mr0 add-flow $udp_r $udp_s $eface0_mr0 2 ;#2000.

$mn1 attach-agent $udp_r $eface0_mn1 3
$handover_mr1 add-flow $udp_r $udp_s $eface0_mr1 2 ;#2000.

$udp_s set packetSize_ 1000
$udp_r set packetSize_ 1000
$udp_server set packetSize_ 1000

$udp_mr0	set packetSize_ 1000
$udp_mr0_ha1 set packetSize_ 1000
$udp_mr0_ha2 set packetSize_ 1000
$udp_mr1 set packetSize_ 1000
$udp_mr1_ha1 set packetSize_ 1000
$udp_mr1_ha2 set packetSize_ 1000

$udp_mn0 set packetSize_ 1000

#Setup a MM Application
set mysipapp_s [new Application/mysipApp]
set mysipapp_r [new Application/mysipApp]
set mysipapp_server [new Application/mysipApp]
$mysipapp_s attach-agent $udp_s
$mysipapp_r attach-agent $udp_r
$mysipapp_server attach-agent $udp_server

#$mysipapp_r set_addr [$eface0_mn0 node-addr]
#change
$mysipapp_r set_addr [$eface0_mn1 node-addr]

$mysipapp_s set pktsize_ 1000
$mysipapp_s set random_ false
$mysipapp_s myID_URL 1000 1.0.0
#$mysipapp_r myID_URL 9999 2.0.0
#$mysipapp_server myID_URL 88 2.0.0
#$mysipapp_server add_URL_record 9999  2.0.0 2.0.129

#testing invite CN
#$mysipapp_server add_URL_record 9999  5.0.0 5.0.129
$mysipapp_server add_URL_record 1000  5.0.3 1.0.0
#$ns at 55 "$mipv6_cn0 binding"
$ns at 55 "$udp_s registration"

$mysipapp_r myID_URL 8888 5.0.0
$mysipapp_server myID_URL 88 5.0.0
$mysipapp_server add_URL_record 9999  5.0.0 5.0.129

$mysipapp_s log_file log handoff



puts " time [expr $moveStart+80]"
#$ns at [expr $moveStart+18] "$mysipapp_r target [$iface1 entry]"
#$ns at [expr $moveStart] "$mysipapp_r target [$iface0 entry]"

#--------------- sem ok start
#$ns at [expr $moveStart+1] "$mysipapp_r registration 0.0.2"
#$ns at [expr $moveStart+5] "$mysipapp_s send_invite 9999 2.0.0"
#$ns at [expr $moveStart+15] "$mysipapp_r registration 3.0.1"
#--------------- sem ok end

#--------------- sem test start
#$ns at 0 "$mipv6_cn binding"
#$ns at [expr $moveStart+2] "$mysipapp_r registration 0.0.2"
#$ns at [expr $moveStart+1] "$handover binding $iface0"
#$ns at [expr $moveStart+5] "$mysipapp_s send_invite 9999 5.0.2"
#$ns at [expr $moveStart+15] "$handover binding $iface1"
#$ns at [expr $moveStart+15] "$mysipapp_r registration 3.0.1"
#--------------- sem test end

#$handover set-ha 5.0.0 5.0.2
#$handover set-nemo-prefix 6.0.0

$ns at 60 "$mysipapp_r send_invite 1000 5.0.0"
#$ns at 87 "$mysipapp_s send_invite 9999 5.0.1"
$ns at [expr $moveStop - 40] "$mysipapp_s dump_handoff_info" 




#$ns at [expr $moveStart+20] "$mysipapp_r send_invite 1000 1.0.0"

#$ns at [expr $moveStart+25] "$mysipapp_r send_re_invite 1000 1.0.0"
#$ns at [expr $moveStart+22] "[$iface1 set dmux_] install 3 $udp_s"


#Start the application 1sec before the MN is entering the WLAN cell
#$ns at [expr $moveStart - 1] "$cbr_ start"
#$ns at 0 "$cbr_ start"

#Stop the application according to another poisson distribution (note that we don't leave the 802.11 cell)
#$ns at [expr $moveStop  + 1] "$cbr_ stop"

# set original status of interface. By default they are up..so to have a link up, 
# we need to put them down first.
$ns at 0 "[eval $eface0_mr0 set mac_(2)] disconnect-link" ;#UMTS UE
$ns at 0.1 "[eval $eface0_mr0 set mac_(2)] connect-link"     ;#umts link 

$ns at $moveStart "puts \"At $moveStart Mobile Node starts moving\""
#$ns at [expr $moveStart+10] "puts \"++At [expr $moveStart+10] Mobile Node enters wlan\""
#$ns at [expr $moveStart+110] "puts \"++At [expr $moveStart+110] Mobile Node leaves wlan\""
$ns at $moveStop "puts \"Mobile Node stops moving\""
$ns at [expr $moveStop - 40] "puts \"Simulation ends at [expr $moveStop+10]\"" 
$ns at [expr $moveStop - 40] "finish" 

if {$quiet == 0} {
puts " Simulation is running ... please wait ..."
}

$ns run
