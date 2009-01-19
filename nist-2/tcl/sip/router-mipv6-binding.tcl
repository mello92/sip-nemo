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

#	1	:	tunnel-in-tunnel	
#	2	:	2-tunnel
Agent/MIHUser/IFMNGMT/MIPV6 set exp_mr_ 9	

#Agent/MIHUser/IFMNGMT/MIPV6 set mr_bs_ 1

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
#Mac/802_11 set debug_ 0
Mac/802_11 set debug_ 0
Agent/MIHUser/IFMNGMT/MIPV6/Handover/Handover1 set confidence_th_ 100

Agent/NEMO set debug_ 1

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
AddrParams set domain_num_  36                      ;# domain number
AddrParams set cluster_num_ {1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1}   ;# cluster number for each domain 
																															 #10                                              20                                                  30                            36

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
lappend tmp 1                                      ;# iface0_mr0 eface_mn0 ----------edit multiple
lappend tmp 1                                      ;# mn0
lappend tmp 1                                      ;# mr0_ha1
lappend tmp 1                                      ;# mn0_ha2
lappend tmp 2                                      ;# bs_eface2_mr0 eface2_mr0

lappend tmp 1                                      ;# mr1_ha1
lappend tmp 1                                      ;# mr1_ha2
lappend tmp 2                                      ;# bs_eface2_mr1 eface2_mr1
lappend tmp 2                                      ;# bs_eface1_mr1 eface1_mr1
lappend tmp 1                                      ;# iface0_mr1 eface0_mn1 ----------edit multiple
lappend tmp 1                                      ;# mn1
lappend tmp 1                                      ;# mr1
lappend tmp 2                                      ;# mface0_mr0 mface0_mr0_lan
lappend tmp 3                                      ;# mface0_mr1 mface0_mr1_lan ----------edit multiple
lappend tmp 1                                      ;# mr_router
lappend tmp 2                                      ;# mface0_mr_bs mface0_mr_bs_lan 
lappend tmp 1                                      ;# iface0_mr_bs
lappend tmp 1                                      ;# mr_bs

lappend tmp 1                                      ;# mr2_ha1
lappend tmp 1                                      ;# mr3_ha1
lappend tmp 2                                      ;# bs_eface1_mr2 eface1_mr2
lappend tmp 2                                      ;# bs_eface1_mr3 eface1_mr3
lappend tmp 1                                      ;# mr2
lappend tmp 1                                      ;# mr3
lappend tmp 2                                      ;# mface0_mr2 mface0_mr2_lan
lappend tmp 3                                      ;# mface0_mr3 mface0_mr3_lan ----------edit multiple
lappend tmp 1                                      ;# mr_router2
lappend tmp 2                                      ;# mface0_mr_bs2 mface0_mr_bs2_lan
lappend tmp 1                                      ;# iface0_mr_bs2
lappend tmp 1                                      ;# mr_bs2


AddrParams set nodes_num_ $tmp

array set node_type {MN 0 MN_HA 1 MR 2 MR_HA 3 CN 4 MR_BS 5}

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
set mr_router [$ns node 20.0.0]

set mr2_ha1 [$ns node 24.0.0]
set mr3_ha1 [$ns node 25.0.0]
set mr_router2 [$ns node 32.0.0]

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

set mface0_mr2 [$ns node 30.0.0]
lappend nodelist3 $mface0_mr2
set mface0_mr2_lan [$ns newLan $nodelist3 10Mb 1ms \
 -llType LL -ifqType Queue/DropTail \
 -macType Mac/802_3 -chanType Channel -address 30.0.1]

set mface0_mr3 [$ns node 31.0.0]
lappend nodelist4 $mface0_mr3
set mface0_mr3_lan [$ns newLan $nodelist4 10Mb 1ms \
 -llType LL -ifqType Queue/DropTail \
 -macType Mac/802_3 -chanType Channel -address 31.0.1]

set mface0_mr_bs [$ns node 21.0.0]
lappend nodelist5 $mface0_mr_bs
set mface0_mr_bs_lan [$ns newLan $nodelist5 10Mb 1ms \
 -llType LL -ifqType Queue/DropTail \
 -macType Mac/802_3 -chanType Channel -address 21.0.1]

set mface0_mr_bs2 [$ns node 33.0.0]
lappend nodelist6 $mface0_mr_bs2
set mface0_mr_bs2_lan [$ns newLan $nodelist6 10Mb 1ms \
 -llType LL -ifqType Queue/DropTail \
 -macType Mac/802_3 -chanType Channel -address 33.0.1]


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
	puts "mface0_mr_bs: tcl=$mface0_mr_bs; id=[$mface0_mr_bs id]; addr=[$mface0_mr_bs node-addr]"
	puts "mr_router: tcl=$mr_router; id=[$mr_router id]; addr=[$mr_router node-addr]"
	
	puts "mr2_ha1: tcl=$mr2_ha1; id=[$mr2_ha1 id]; addr=[$mr2_ha1 node-addr]"
	puts "mr3_ha1: tcl=$mr3_ha1; id=[$mr3_ha1 id]; addr=[$mr3_ha1 node-addr]"
	puts "mr_router2: tcl=$mr_router2; id=[$mr_router2 id]; addr=[$mr_router2 node-addr]"
	puts "mface0_mr2: tcl=$mface0_mr2; id=[$mface0_mr2 id]; addr=[$mface0_mr2 node-addr]"
	puts "mface0_mr3: tcl=$mface0_mr3; id=[$mface0_mr3 id]; addr=[$mface0_mr3 node-addr]"
	puts "mface0_mr_bs2: tcl=$mface0_mr_bs2; id=[$mface0_mr_bs2 id]; addr=[$mface0_mr_bs2 node-addr]"
	
}

# connect links 
$ns duplex-link $rnc $router1 622Mbit 0.4ms DropTail 1000
$ns duplex-link $router1 $cn0 100MBit 30ms DropTail 1000
$ns duplex-link $router1 $ha 100MBit 30ms DropTail 1000
$ns duplex-link $router1 $mr0_ha1 100MBit 30ms DropTail 1000
$ns duplex-link $router1 $mr0_ha2 100MBit 30ms DropTail 1000
$ns duplex-link $router1 $mr1_ha1 100MBit 30ms DropTail 1000
$ns duplex-link $router1 $mr1_ha2 100MBit 30ms DropTail 1000
#$ns duplex-link $mface0_mr0 $mr_router 100MBit 30ms DropTail 1000
$ns duplex-link $mface0_mr1 $mr_router 100MBit 30ms DropTail 1000
$ns duplex-link $mface0_mr_bs $mr_router 100MBit 30ms DropTail 1000

$ns duplex-link $router1 $mr2_ha1 100MBit 30ms DropTail 1000
$ns duplex-link $router1 $mr3_ha1 100MBit 30ms DropTail 1000
#$ns duplex-link $mface0_mr2 $mr_router2 100MBit 30ms DropTail 1000
$ns duplex-link $mface0_mr3 $mr_router2 100MBit 30ms DropTail 1000
$ns duplex-link $mface0_mr_bs2 $mr_router2 100MBit 30ms DropTail 1000


$rnc add-gateway $router1

# creation of the MutiFaceNodes. It MUST be done before the 802.11
$ns node-config  -multiIf ON                            ;#to create MultiFaceNode 

set mr0 [$ns node 4.0.0] 
set mn0 [$ns node 7.0.0]
set mr1 [$ns node 17.0.0] 
set mn1 [$ns node 16.0.0]
set mr_bs [$ns node 23.0.0]

set mr2 [$ns node 28.0.0] 
set mr3 [$ns node 29.0.0] 
set mr_bs2 [$ns node 35.0.0] 


$ns node-config  -multiIf OFF                           ;#reset attribute
if {$quiet == 0} {
	puts "mr0: tcl=$mr0; id=[$mr0 id]; addr=[$mr0 node-addr]"
	puts "mr1: tcl=$mr1; id=[$mr1 id]; addr=[$mr1 node-addr]"
	puts "mn0: tcl=$mn0; id=[$mn0 id]; addr=[$mn0 node-addr]"
	puts "mn1: tcl=$mn1; id=[$mn1 id]; addr=[$mn1 node-addr]"
	puts "mr_bs: tcl=$mr_bs; id=[$mr_bs id]; addr=[$mr_bs node-addr]"
	
	puts "mr2: tcl=$mr2; id=[$mr2 id]; addr=[$mr2 node-addr]"
	puts "mr3: tcl=$mr3; id=[$mr3 id]; addr=[$mr3 node-addr]"
	puts "mr_bs2: tcl=$mr_bs2; id=[$mr_bs2 id]; addr=[$mr_bs2 node-addr]"
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
create-god 54				                ;# give the number of nodes 


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


set iface0_mr0 [$ns node 6.0.0]     ;# node id is 8. 
$iface0_mr0 random-motion 0		;# disable random motion
#$iface0_mr0 base-station [AddrParams addr2id [$iface0_mr1 node-addr]] ;#attach mn to basestation
$iface0_mr0 set X_ 200
$iface0_mr0 set Y_ 200
$iface0_mr0 set Z_ 0.0

if {$quiet == 0} {
	puts "iface0_mr0: tcl=$iface0_mr0; id=[$iface0_mr0 id]; addr=[$iface0_mr0 node-addr]"
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


set iface0_mr1 [$ns node 15.0.0]     ;# node id is 8. 
$iface0_mr1 random-motion 0		;# disable random motion
#$iface0_mr1 base-station [AddrParams addr2id [$iface0_mr0 node-addr]] ;#attach mn to basestation
#$iface0_mr0 base-station [AddrParams addr2id [$iface0_mr1 node-addr]] ;#attach mn to basestation
$iface0_mr1 set X_ 200
$iface0_mr1 set Y_ 200
$iface0_mr1 set Z_ 0.0

if {$quiet == 0} {
	puts "iface0_mr1: tcl=$iface0_mr1; id=[$iface0_mr1 id]; addr=[$iface0_mr1 node-addr]"
}
#[$iface0_mr0 getMac 0] set-channel 2
set iface0_mr1_Mac [$iface0_mr1 getMac 0]
set AP_ADDR_1 [$iface0_mr1_Mac id]
if {$quiet == 0} {
	puts "bss_id for iface0_mr1 =$AP_ADDR_1"
}
$iface0_mr1_Mac bss_id $AP_ADDR_1
$iface0_mr1_Mac enable-beacon
$iface0_mr1_Mac set-channel 5


#######################
#		configure mr_bs
set iface0_mr_bs [$ns node 22.0.0]     ;# node id is 8. 
$iface0_mr_bs random-motion 0		;# disable random motion
#$iface0_mr1 base-station [AddrParams addr2id [$iface0_mr0 node-addr]] ;#attach mn to basestation
#$iface0_mr0 base-station [AddrParams addr2id [$iface0_mr1 node-addr]] ;#attach mn to basestation
$iface0_mr_bs set X_ 200
$iface0_mr_bs set Y_ 200
$iface0_mr_bs set Z_ 0.0

if {$quiet == 0} {
	puts "iface0_mr_bs: tcl=$iface0_mr_bs; id=[$iface0_mr_bs id]; addr=[$iface0_mr_bs node-addr]"
}
#[$iface0_mr0 getMac 0] set-channel 2
set iface0_mr_bs_Mac [$iface0_mr_bs getMac 0]
set AP_ADDR_1 [$iface0_mr_bs_Mac id]
if {$quiet == 0} {
	puts "bss_id for iface0_mr_bs =$AP_ADDR_1"
}
$iface0_mr_bs_Mac bss_id $AP_ADDR_1
$iface0_mr_bs_Mac enable-beacon
$iface0_mr_bs_Mac set-channel 7


#######################
#		configure mr 2
# configure Base station 802.11
set bs_eface1_mr2 [$ns node 26.0.0]
#$bs_eface1_mr2 set X_ 100.0
$bs_eface1_mr2 set X_ 50.0
$bs_eface1_mr2 set Y_ 100.0
$bs_eface1_mr2 set Z_ 0.0
if {$quiet == 0} {
	puts "bs_eface1_mr2: tcl=$bs_eface1_mr2; id=[$bs_eface1_mr2 id]; addr=[$bs_eface1_mr2 node-addr]"
}
# we need to set the BSS for the base station
set bs_eface1_mr2_Mac [$bs_eface1_mr2 getMac 0]
set AP_ADDR_0 [$bs_eface1_mr2_Mac id]
if {$quiet == 0} {
	puts "bss_id for bs_eface1_mr2=$AP_ADDR_0"
}
$bs_eface1_mr2_Mac bss_id $AP_ADDR_0
$bs_eface1_mr2_Mac enable-beacon
$bs_eface1_mr2_Mac set-channel 9


#######################
#		configure mr 3
# configure Base station 802.11
set bs_eface1_mr3 [$ns node 27.0.0]
#$bs_eface1_mr3 set X_ 100.0
$bs_eface1_mr3 set X_ 50.0
$bs_eface1_mr3 set Y_ 100.0
$bs_eface1_mr3 set Z_ 0.0
if {$quiet == 0} {
	puts "bs_eface1_mr3: tcl=$bs_eface1_mr3 id=[$bs_eface1_mr3 id]; addr=[$bs_eface1_mr3 node-addr]"
}
# we need to set the BSS for the base station
set bs_eface1_mr3_Mac [$bs_eface1_mr3 getMac 0]
set AP_ADDR_0 [$bs_eface1_mr3_Mac id]
if {$quiet == 0} {
	puts "bss_id for bs_eface1_mr3=$AP_ADDR_0"
}
$bs_eface1_mr3_Mac bss_id $AP_ADDR_0
$bs_eface1_mr3_Mac enable-beacon
$bs_eface1_mr3_Mac set-channel 10


#######################
#		configure mr_bs2
set iface0_mr_bs2 [$ns node 34.0.0]     ;# node id is 8. 
$iface0_mr_bs2 random-motion 0		;# disable random motion
$iface0_mr_bs2 set X_ 200
$iface0_mr_bs2 set Y_ 260
$iface0_mr_bs2 set Z_ 0.0

if {$quiet == 0} {
	puts "iface0_mr_bs2: tcl=$iface0_mr_bs2; id=[$iface0_mr_bs2 id]; addr=[$iface0_mr_bs2 node-addr]"
}
set iface0_mr_bs2_Mac [$iface0_mr_bs2 getMac 0]
set AP_ADDR_1 [$iface0_mr_bs2_Mac id]
if {$quiet == 0} {
	puts "bss_id for iface0_mr_bs2 =$AP_ADDR_1"
}
$iface0_mr_bs2_Mac bss_id $AP_ADDR_1
$iface0_mr_bs2_Mac enable-beacon
$iface0_mr_bs2_Mac set-channel 7


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
	puts "eface1_mr0: tcl=$eface1_mr0; id=[$eface1_mr0 id]; addr=[$eface1_mr0 node-addr]"
}
[$eface1_mr0 getMac 0] set-channel 1


set eface2_mr0 [$ns node 10.0.1]     ;# node id is 8. 
$eface2_mr0 random-motion 0		;# disable random motion
$eface2_mr0 base-station [AddrParams addr2id [$bs_eface2_mr0 node-addr]] ;#attach mn to basestation
$eface2_mr0 set X_ $X_src
$eface2_mr0 set Y_ $Y_src
$eface2_mr0 set Z_ 0.0

if {$quiet == 0} {
	puts "eface2_mr0: tcl=$eface2_mr0; id=[$eface2_mr0 id]; addr=[$eface2_mr0 node-addr]"
}
[$eface2_mr0 getMac 0] set-channel 3


#######################
#		configure mn 0
set eface0_mn0 [$ns node 31.0.2]     ;# node id is 8. 
$eface0_mn0 random-motion 0		;# disable random motion
#$eface0_mn0 base-station [AddrParams addr2id [$iface0_mr0 node-addr]] ;#attach mn to basestation
$eface0_mn0 base-station [AddrParams addr2id [$iface0_mr_bs2 node-addr]] ;#attach mn to basestation
$eface0_mn0 set X_ 200
$eface0_mn0 set Y_ 170
$eface0_mn0 set Z_ 0.0

if {$quiet == 0} {
	puts "eface0_mn0: tcl=$eface0_mn0; id=[$eface0_mn0 id]; addr=[$eface0_mn0 node-addr]"
}
[$eface0_mn0 getMac 0] set-channel 11



#######################
#		configure mr 1
set eface1_mr1 [$ns node 14.0.1]     ;# node id is 8. 
$eface1_mr1 random-motion 0		;# disable random motion
$eface1_mr1 base-station [AddrParams addr2id [$bs_eface1_mr1 node-addr]] ;#attach mn to basestation
$eface1_mr1 set X_ $X_src
$eface1_mr1 set Y_ $Y_src
$eface1_mr1 set Z_ 0.0

if {$quiet == 0} {
	puts "eface1_mr1: tcl=$eface1_mr1; id=[$eface1_mr1 id]; addr=[$eface1_mr1 node-addr]"
}
[$eface1_mr1 getMac 0] set-channel 4


set eface2_mr1 [$ns node 13.0.1]     ;# node id is 8. 
$eface2_mr1 random-motion 0		;# disable random motion
$eface2_mr1 base-station [AddrParams addr2id [$bs_eface2_mr1 node-addr]] ;#attach mn to basestation
$eface2_mr1 set X_ $X_src
$eface2_mr1 set Y_ $Y_src
$eface2_mr1 set Z_ 0.0

if {$quiet == 0} {
	puts "eface2_mr1: tcl=$eface2_mr1; id=[$eface2_mr1 id]; addr=[$eface2_mr1 node-addr]"
}
[$eface2_mr1 getMac 0] set-channel 6


#######################
#		configure mn 1
set eface0_mn1 [$ns node 19.0.2]     ;# node id is 8. 
$eface0_mn1 random-motion 0		;# disable random motion
#$eface0_mn1 base-station [AddrParams addr2id [$iface0_mr1 node-addr]] ;#attach mn to basestation
$eface0_mn1 base-station [AddrParams addr2id [$iface0_mr_bs node-addr]] ;#attach mn to basestation
$eface0_mn1 set X_ 200
$eface0_mn1 set Y_ 200
$eface0_mn1 set Z_ 0.0

if {$quiet == 0} {
	puts "eface0_mn1: tcl=$eface0_mn1; id=[$eface0_mn1 id]; addr=[$eface0_mn1 node-addr]"
}
#[$eface0_mn1 getMac 0] set-channel 5
[$eface0_mn1 getMac 0] set-channel 7


#######################
#		configure mr 2
set eface1_mr2 [$ns node 26.0.1]     ;# node id is 8. 
$eface1_mr2 random-motion 0		;# disable random motion
$eface1_mr2 base-station [AddrParams addr2id [$bs_eface1_mr2 node-addr]] ;#attach mn to basestation
$eface1_mr2 set X_ $X_src
$eface1_mr2 set Y_ $Y_src
$eface1_mr2 set Z_ 0.0

if {$quiet == 0} {
	puts "eface1_mr2: tcl=$eface1_mr2; id=[$eface1_mr2 id]; addr=[$eface1_mr2 node-addr]"
}
[$eface1_mr2 getMac 0] set-channel 9


#######################
#		configure mr 3
set eface1_mr3 [$ns node 27.0.1]     ;# node id is 8. 
$eface1_mr3 random-motion 0		;# disable random motion
$eface1_mr3 base-station [AddrParams addr2id [$bs_eface1_mr3 node-addr]] ;#attach mn to basestation
$eface1_mr3 set X_ $X_src
$eface1_mr3 set Y_ $Y_src
$eface1_mr3 set Z_ 0.0

if {$quiet == 0} {
	puts "eface1_mr3: tcl=$eface1_mr3; id=[$eface1_mr3 id]; addr=[$eface1_mr3 node-addr]"
}
[$eface1_mr3 getMac 0] set-channel 10



#
#	calculate the speed of the node
#
#######################
#		configure mr 0
#$ns at $moveStart "$eface1_mr0 setdest $X_dst $Y_dst $moveSpeed"
#$ns at $moveStart "$eface2_mr0 setdest $X_dst $Y_dst $moveSpeed"
#######################
#		configure mr 1
#$ns at $moveStart "$eface1_mr1 setdest $X_dst $Y_dst $moveSpeed"
#$ns at $moveStart "$eface2_mr1 setdest $X_dst $Y_dst $moveSpeed"
#######################
#		configure mr 3
#$ns at $moveStart "$eface1_mr3 setdest $X_dst $Y_dst $moveSpeed"

#######################
#		configure mn 1
$ns at 70 "$eface0_mn1 setdest 200.0 260.0 60.0"


#
#	add the interfaces to supernode
#
#######################
#		configure mr 0
$mr0 add-interface-node $eface0_mr0
$mr0 add-interface-node $eface1_mr0
#$mr0 add-interface-node $eface2_mr0
$mr0 add-interface-node $iface0_mr0
$mr0 add-interface-node $mface0_mr0
#######################
#		configure mn 0
$mn0 add-interface-node $eface0_mn0
#######################
#		configure mr 1
$mr1 add-interface-node $eface0_mr1
$mr1 add-interface-node $eface1_mr1
$mr1 add-interface-node $eface2_mr1
$mr1 add-interface-node $iface0_mr1
$mr1 add-interface-node $mface0_mr1
#######################
#		configure mn 1
$mn1 add-interface-node $eface0_mn1
#######################
#		configure mr_bs
$mr_bs add-interface-node $iface0_mr_bs
$mr_bs add-interface-node $mface0_mr_bs

#######################
#		configure mr 2
$mr2 add-interface-node $eface1_mr2
$mr2 add-interface-node $mface0_mr2
#######################
#		configure mr 3
$mr3 add-interface-node $eface1_mr3
$mr3 add-interface-node $mface0_mr3
#######################
#		configure mr_bs2
$mr_bs2 add-interface-node $iface0_mr_bs2
$mr_bs2 add-interface-node $mface0_mr_bs2


#
# add link to backbone
#
#######################
#		configure mr 0
$ns duplex-link $bs_eface1_mr0 $router1 100MBit 30ms DropTail 1000
$ns duplex-link $bs_eface2_mr0 $router1 100MBit 30ms DropTail 1000
#######################
#		configure mr 1
$ns duplex-link $bs_eface1_mr1 $router1 100MBit 30ms DropTail 1000
$ns duplex-link $bs_eface2_mr1 $router1 100MBit 30ms DropTail 1000

#######################
#		configure mr 2
$ns duplex-link $bs_eface1_mr2 $router1 100MBit 30ms DropTail 1000
#######################
#		configure mr 3
$ns duplex-link $bs_eface1_mr3 $router1 100MBit 30ms DropTail 1000


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

#######################
#		configure mr 2
set nd_bs_eface1_mr2 [$bs_eface1_mr2 install-nd]
$nd_bs_eface1_mr2 set-router TRUE
$nd_bs_eface1_mr2 router-lifetime 18
$ns at 1 "$nd_bs_eface1_mr2 start-ra"

set mipv6_bs_eface1_mr2 [$bs_eface1_mr2 install-default-ifmanager]
set mih_bs_eface1_mr2 [$bs_eface1_mr2 install-mih]
$mipv6_bs_eface1_mr2 connect-mih $mih_bs_eface1_mr2
set tmp2 [$bs_eface1_mr2 set mac_(0)] ;#in 802.11 one interface is created
$tmp2 mih $mih_bs_eface1_mr2
$mih_bs_eface1_mr2 add-mac $tmp2

#######################
#		configure mr 3
set nd_bs_eface1_mr3 [$bs_eface1_mr3 install-nd]
$nd_bs_eface1_mr3 set-router TRUE
$nd_bs_eface1_mr3 router-lifetime 18
$ns at 1 "$nd_bs_eface1_mr3 start-ra"

set mipv6_bs_eface1_mr3 [$bs_eface1_mr3 install-default-ifmanager]
set mih_bs_eface1_mr3 [$bs_eface1_mr3 install-mih]
$mipv6_bs_eface1_mr3 connect-mih $mih_bs_eface1_mr3
set tmp2 [$bs_eface1_mr3 set mac_(0)] ;#in 802.11 one interface is created
$tmp2 mih $mih_bs_eface1_mr3
$mih_bs_eface1_mr3 add-mac $tmp2


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
#$ns at 5 "$nd_iface0_mr0 start-ra"

set nd_mface0_mr0 [$mface0_mr0 install-nd]
$nd_mface0_mr0 set-router TRUE
$nd_mface0_mr0 router-lifetime 180
$nd_mface0_mr0 enable-broadcast FALSE
#$nd_mface0_mr0 add-ra-target 19.0.0
#$nd_mface0_mr0 add-ra-target 21.0.0
$ns at 10 "$nd_mface0_mr0 start-ra"

#######################
#		configure mr 1
set nd_eface1_mr1 [$eface1_mr1 install-nd]
set nd_eface2_mr1 [$eface2_mr1 install-nd]

set nd_iface0_mr1 [$iface0_mr1 install-nd]
$nd_iface0_mr1 set-router TRUE
$nd_iface0_mr1 router-lifetime 18
$ns at 5 "$nd_iface0_mr1 start-ra"

set nd_mface0_mr1 [$mface0_mr1 install-nd]
$nd_mface0_mr1 set-router TRUE
$nd_mface0_mr1 router-lifetime 18
$nd_mface0_mr1 enable-broadcast FALSE
#$nd_mface0_mr1 add-ra-target 18.0.0
$nd_mface0_mr1 add-ra-target 21.0.0
$ns at 10 "$nd_mface0_mr1 start-ra"

#######################
#		configure mr_bs
set nd_iface0_mr_bs [$iface0_mr_bs install-nd]
$nd_iface0_mr_bs set-router TRUE
$nd_iface0_mr_bs set-mr-bs TRUE
$nd_iface0_mr_bs router-lifetime 18
$ns at 15 "$nd_iface0_mr_bs start-ra"

set nd_mface0_mr_bs [$mface0_mr_bs install-nd]
$nd_mface0_mr_bs set-mr-bs TRUE

#######################
#		configure mr 2
set nd_eface1_mr2 [$eface1_mr2 install-nd]

set nd_mface0_mr2 [$mface0_mr2 install-nd]
$nd_mface0_mr2 set-router TRUE
$nd_mface0_mr2 router-lifetime 18
$nd_mface0_mr2 enable-broadcast FALSE
#$nd_mface0_mr2 add-ra-target 31.0.0
#$nd_mface0_mr2 add-ra-target 33.0.0
$ns at 10 "$nd_mface0_mr2 start-ra"

#######################
#		configure mr 3
set nd_eface1_mr3 [$eface1_mr3 install-nd]

set nd_mface0_mr3 [$mface0_mr3 install-nd]
$nd_mface0_mr3 set-router TRUE
$nd_mface0_mr3 router-lifetime 180
$nd_mface0_mr3 enable-broadcast FALSE
#$nd_mface0_mr3 add-ra-target 30.0.0
$nd_mface0_mr3 add-ra-target 33.0.0
$ns at 10 "$nd_mface0_mr3 start-ra"

#######################
#		configure mr_bs2
set nd_iface0_mr_bs2 [$iface0_mr_bs2 install-nd]
$nd_iface0_mr_bs2 set-router TRUE
$nd_iface0_mr_bs2 set-mr-bs TRUE
$nd_iface0_mr_bs2 router-lifetime 18
$ns at 15 "$nd_iface0_mr_bs2 start-ra"
$ns at 71.2 "$nd_iface0_mr_bs2 send-ads"

set nd_mface0_mr_bs2 [$mface0_mr_bs2 install-nd]
$nd_mface0_mr_bs2 set-mr-bs TRUE


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
set nemo_mface0_mr0 [$mr0 install-nemo 204]

$nemo_eface0_mr0 connect-interface $eface0_mr0
$nemo_eface1_mr0 connect-interface $eface1_mr0
$nemo_eface2_mr0 connect-interface $eface2_mr0
$nemo_iface0_mr0 connect-interface $iface0_mr0
$nemo_mface0_mr0 connect-interface $mface0_mr0

#######################
#		configure mr 1
set nemo_eface0_mr1 [$mr1 install-nemo 200]
set nemo_eface1_mr1 [$mr1 install-nemo 201]
set nemo_eface2_mr1 [$mr1 install-nemo 202]
set nemo_iface0_mr1 [$mr1 install-nemo 203]
set nemo_mface0_mr1 [$mr1 install-nemo 204]

$nemo_eface0_mr1 connect-interface $eface0_mr1
$nemo_eface1_mr1 connect-interface $eface1_mr1
$nemo_eface2_mr1 connect-interface $eface2_mr1
$nemo_iface0_mr1 connect-interface $iface0_mr1
$nemo_mface0_mr1 connect-interface $mface0_mr1

#######################
#		configure mr_bs
set nemo_iface0_mr_bs [$mr_bs install-nemo 200]
set nemo_mface0_mr_bs [$mr_bs install-nemo 201]

$nemo_iface0_mr_bs connect-interface $iface0_mr_bs
$nemo_mface0_mr_bs connect-interface $mface0_mr_bs

#######################
#		configure mr 2
set nemo_eface1_mr2 [$mr2 install-nemo 200]
set nemo_mface0_mr2 [$mr2 install-nemo 201]

$nemo_eface1_mr2 connect-interface $eface1_mr2
$nemo_mface0_mr2 connect-interface $mface0_mr2

#######################
#		configure mr 3
set nemo_eface1_mr3 [$mr3 install-nemo 200]
set nemo_mface0_mr3 [$mr3 install-nemo 201]

$nemo_eface1_mr3 connect-interface $eface1_mr3
$nemo_mface0_mr3 connect-interface $mface0_mr3

#######################
#		configure mr_bs2
set nemo_iface0_mr_bs2 [$mr_bs2 install-nemo 200]
set nemo_mface0_mr_bs2 [$mr_bs2 install-nemo 201]

$nemo_iface0_mr_bs2 connect-interface $iface0_mr_bs2
$nemo_mface0_mr_bs2 connect-interface $mface0_mr_bs2


#######################
#		configure mn 0
set nemo_eface0_mn0 [$mn0 install-nemo 200]

$nemo_eface0_mn0 connect-interface $eface0_mn0

#######################
#		configure mn 1
set nemo_eface0_mn1 [$mn1 install-nemo 200]

$nemo_eface0_mn1 connect-interface $eface0_mn1


#
# install Interface Management modules (MIPv6 or Handover)
#

# MN
#######################
#		configure mn 0
set mipv6_mn0 [$mn0 install-default-ifmanager]
$nd_eface0_mn0 set-ifmanager $mipv6_mn0
$mipv6_mn0 set-node-type $node_type(MN)
$mipv6_mn0 set-mn 5.0.0 5.0.1 $nemo_eface0_mn0

$mipv6_mn0 set-mr-bs-daddr 34.0.0

#######################
#		configure mn 1
set mipv6_mn1 [$mn1 install-default-ifmanager]
$nd_eface0_mn1 set-ifmanager $mipv6_mn1
$mipv6_mn1 set-node-type $node_type(MN)
$mipv6_mn1 set-mn 5.0.0 5.0.2 $nemo_eface0_mn1

$mipv6_mn1 set-mr-bs-daddr 22.0.0


# MR
#######################
#		configure mr 0
set handover_mr0 [new Agent/MIHUser/IFMNGMT/MIPV6/Handover/Handover1]
$mr0 install-ifmanager $handover_mr0

$nd_eface1_mr0 set-ifmanager $handover_mr0 
$nd_eface2_mr0 set-ifmanager $handover_mr0 
$nd_eface0_mr0 set-ifmanager $handover_mr0 

$nd_iface0_mr0 set-ifmanager $handover_mr0 
$nd_mface0_mr0 set-ifmanager $handover_mr0 

$handover_mr0 set-node-type $node_type(MR)

#(1,1,n)
#$handover_mr0 set-mr 8.0.0 8.0.1 6.0.0 $nemo_eface1_mr0 $nemo_iface0_mr0
#$handover_mr0 set-mr 8.0.0 8.0.2 100.0.0 $nemo_eface2_mr0 $nemo_iface0_mr0

#(1,n,1)
#$handover_mr0 set-mr 8.0.0 8.0.1 6.0.0 $nemo_eface1_mr0 $nemo_iface0_mr0
#$handover_mr0 set-mr 9.0.0 8.0.2 100.0.0 $nemo_eface2_mr0 $nemo_iface0_mr0

#(1,n,n)
#$handover_mr0 set-mr 8.0.0 8.0.3 6.0.0 $nemo_iface0_mr0 $nemo_iface0_mr0

$handover_mr0 set-mr 8.0.0 8.0.1 18.0.0 $nemo_eface1_mr0 $nemo_mface0_mr0
#$handover_mr0 set-mr 9.0.0 8.0.2 100.0.0 $nemo_eface2_mr0 $nemo_iface0_mr0

$handover_mr0 set-mface [$mface0_mr1 node-addr] [$mface0_mr0 node-addr] $nemo_mface0_mr0

$handover_mr0 set-mr-bs-daddr 21.0.0

#######################
#		configure mr 1
set handover_mr1 [new Agent/MIHUser/IFMNGMT/MIPV6/Handover/Handover1]
$mr1 install-ifmanager $handover_mr1

$nd_eface1_mr1 set-ifmanager $handover_mr1 
$nd_eface2_mr1 set-ifmanager $handover_mr1 
$nd_eface0_mr1 set-ifmanager $handover_mr1 

$nd_iface0_mr1 set-ifmanager $handover_mr1
$nd_mface0_mr1 set-ifmanager $handover_mr1

$handover_mr1 set-node-type $node_type(MR)

#(1,1,n)
#$handover_mr1 set-mr 11.0.0 11.0.1 15.0.0 $nemo_eface1_mr1 $nemo_iface0_mr1
#$handover_mr1 set-mr 11.0.0 11.0.2 101.0.0 $nemo_eface2_mr1 $nemo_iface0_mr1

#(1,n,1)
#$handover_mr1 set-mr 11.0.0 11.0.1 15.0.0 $nemo_eface1_mr1 $nemo_iface0_mr1
#$handover_mr1 set-mr 12.0.0 11.0.2 101.0.0 $nemo_eface2_mr1 $nemo_iface0_mr1

#(1,n,n)
#$handover_mr1 set-mr 11.0.0 11.0.3 15.0.0 $nemo_iface0_mr1 $nemo_iface0_mr1

$handover_mr1 set-mr 11.0.0 11.0.1 19.0.0 $nemo_eface1_mr1 $nemo_mface0_mr1
$handover_mr1 set-mr 12.0.0 11.0.2 101.0.0 $nemo_eface2_mr1 $nemo_iface0_mr1

$handover_mr1 set-mface [$mface0_mr0 node-addr] [$mface0_mr1 node-addr] $nemo_mface0_mr1

$handover_mr1 set-mr-bs-daddr 21.0.0


#######################
#		configure mr 2
set handover_mr2 [new Agent/MIHUser/IFMNGMT/MIPV6/Handover/Handover1]
$mr2 install-ifmanager $handover_mr2

$nd_eface1_mr2 set-ifmanager $handover_mr2 
$nd_mface0_mr2 set-ifmanager $handover_mr2

$handover_mr2 set-node-type $node_type(MR)

$handover_mr2 set-mr 24.0.0 24.0.1 30.0.0 $nemo_eface1_mr2 $nemo_mface0_mr2

$handover_mr2 set-mface [$mface0_mr3 node-addr] [$mface0_mr2 node-addr] $nemo_mface0_mr2

$handover_mr2 set-mr-bs-daddr 33.0.0

#######################
#		configure mr 3
set handover_mr3 [new Agent/MIHUser/IFMNGMT/MIPV6/Handover/Handover1]
$mr3 install-ifmanager $handover_mr3

$nd_eface1_mr3 set-ifmanager $handover_mr3 
$nd_mface0_mr3 set-ifmanager $handover_mr3

$handover_mr3 set-node-type $node_type(MR)

$handover_mr3 set-mr 25.0.0 25.0.1 31.0.0 $nemo_eface1_mr3 $nemo_mface0_mr3

$handover_mr3 set-mface [$mface0_mr2 node-addr] [$mface0_mr3 node-addr] $nemo_mface0_mr3

$handover_mr3 set-mr-bs-daddr 33.0.0


# MR_BS
#######################
#		configure mr_bs
set mipv6_mr_bs	[$mr_bs install-default-ifmanager]

$nd_iface0_mr_bs set-ifmanager $mipv6_mr_bs
$nd_mface0_mr_bs set-ifmanager $mipv6_mr_bs

$mipv6_mr_bs set-node-type $node_type(MR_BS)

$mipv6_mr_bs set-mr-bs $nemo_mface0_mr_bs $nemo_iface0_mr_bs 

# MR_BS2
#######################
#		configure mr_bs2
set mipv6_mr_bs2	[$mr_bs2 install-default-ifmanager]

$nd_iface0_mr_bs2 set-ifmanager $mipv6_mr_bs2
$nd_mface0_mr_bs2 set-ifmanager $mipv6_mr_bs2

$mipv6_mr_bs2 set-node-type $node_type(MR_BS)

$mipv6_mr_bs2 set-mr-bs $nemo_mface0_mr_bs2 $nemo_iface0_mr_bs2


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

#######################
#		configure mr 2
set mipv6_mr2_ha1	[$mr2_ha1 install-default-ifmanager]
$mipv6_mr2_ha1 set-node-type $node_type(MR_HA)

#######################
#		configure mr 3
set mipv6_mr3_ha1	[$mr3_ha1 install-default-ifmanager]
$mipv6_mr3_ha1 set-node-type $node_type(MR_HA)


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

$handover_mr0 nd_mac $nd_mface0_mr0 [$mface0_mr0 set mac_(0)]

#######################
#		configure mr 1
set mih_mr1 [$mr1 install-mih]	;# install MIH in multi-interface node
$handover_mr1 connect-mih $mih_mr1 ;#create connection between MIH and iface management
$handover_mr1 nd_mac $nd_eface1_mr1 [$eface1_mr1 set mac_(0)]
$handover_mr1 nd_mac $nd_eface2_mr1 [$eface2_mr1 set mac_(0)]

$handover_mr1 nd_mac $nd_mface0_mr1 [$mface0_mr1 set mac_(0)]

#######################
#		configure mr_bs
$mipv6_mr_bs nd_mac $nd_iface0_mr_bs [$iface0_mr_bs set mac_(0)]
$mipv6_mr_bs nd_mac $nd_mface0_mr_bs [$mface0_mr_bs set mac_(0)]

#######################
#		configure mr 2
set mih_mr2 [$mr2 install-mih]	;# install MIH in multi-interface node
$handover_mr2 connect-mih $mih_mr2 ;#create connection between MIH and iface management
$handover_mr2 nd_mac $nd_eface1_mr2 [$eface1_mr2 set mac_(0)]
$handover_mr2 nd_mac $nd_mface0_mr2 [$mface0_mr2 set mac_(0)]

#######################
#		configure mr 3
set mih_mr3 [$mr3 install-mih]	;# install MIH in multi-interface node
$handover_mr3 connect-mih $mih_mr3 ;#create connection between MIH and iface management
$handover_mr3 nd_mac $nd_eface1_mr3 [$eface1_mr3 set mac_(0)]
$handover_mr3 nd_mac $nd_mface0_mr3 [$mface0_mr3 set mac_(0)]

#######################
#		configure mr_bs2
$mipv6_mr_bs2 nd_mac $nd_iface0_mr_bs2 [$iface0_mr_bs2 set mac_(0)]
$mipv6_mr_bs2 nd_mac $nd_mface0_mr_bs2 [$mface0_mr_bs2 set mac_(0)]


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

set tmp2 [$mface0_mr0 set mac_(0)] ;#in 802.3 one interface is created
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

set tmp2 [$mface0_mr1 set mac_(0)] ;#in 802.11 one interface is created
$tmp2 mih $mih_mr1
$mih_mr1 add-mac $tmp2

#######################
#		configure mr 2
set tmp2 [$eface1_mr2 set mac_(0)] ;#in 802.11 one interface is created
$tmp2 mih $mih_mr2
$mih_mr2 add-mac $tmp2
set tmp2 [$mface0_mr2 set mac_(0)] ;#in 802.11 one interface is created
$tmp2 mih $mih_mr2
$mih_mr2 add-mac $tmp2

#######################
#		configure mr 3
set tmp2 [$eface1_mr3 set mac_(0)] ;#in 802.11 one interface is created
$tmp2 mih $mih_mr3
$mih_mr3 add-mac $tmp2
set tmp2 [$mface0_mr3 set mac_(0)] ;#in 802.11 one interface is created
$tmp2 mih $mih_mr3
$mih_mr3 add-mac $tmp2


#
#create traffic: mysip application between udp_s and udp_r
#

#	Setup a MM UDP connection
set udp_s [new Agent/UDP/Udpmysip]
set udp_r [new Agent/UDP/Udpmysip]
set udp_server [new Agent/UDP/Udpmysip]

# edit for v-to-v
$cn0 attach $udp_s 3

$ha attach $udp_server 3

# edit for v-to-v
$udp_s set-mipv6 $mipv6_cn0
#$udp_s set-mipv6 $mipv6_mn0

$udp_r set-mipv6 $mipv6_mn1

#	edit for binding
#$mipv6_cn0 set-udpmysip $udp_s

# edit for v-to-v
#$mn0 attach-agent $udp_s $eface0_mn0 3
#$handover_mr0 add-flow $udp_s $udp_r $eface0_mr0 2 ;#2000.

$mn1 attach-agent $udp_r $eface0_mn1 3
$handover_mr1 add-flow $udp_r $udp_s $eface0_mr1 2 ;#2000.

$udp_s set packetSize_ 1000
$udp_r set packetSize_ 1000
$udp_server set packetSize_ 1000


#Setup a MM Application
set mysipapp_s [new Application/mysipApp]
set mysipapp_r [new Application/mysipApp]
set mysipapp_server [new Application/mysipApp]
$mysipapp_s attach-agent $udp_s
$mysipapp_r attach-agent $udp_r
$mysipapp_server attach-agent $udp_server

# edit for v-to-v
#$mysipapp_s set_addr [$eface0_mn0 node-addr]

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
$ns at 55 "$mipv6_cn0 binding"


$mysipapp_r myID_URL 9999 5.0.0
$mysipapp_server myID_URL 88 5.0.0
$mysipapp_server add_URL_record 9999  5.0.0 5.0.129

# edit for v-to-i
#$mysipapp_s log_file log handoff
$mysipapp_r log_file log handoff



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

# edit for v-to-i
#$ns at 65 "$mysipapp_r send_invite 1000 5.0.3"
#$ns at [expr $moveStop - 40] "$mysipapp_s dump_handoff_info" 

$ns at 65 "$mysipapp_s send_invite 9999 5.0.2"
$ns at [expr $moveStop - 40] "$mysipapp_r dump_handoff_info" 


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
