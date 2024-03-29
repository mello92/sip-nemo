#
# Copyright (C) 1997 by USC/ISI
# All rights reserved.                                            
#                                                                
# Redistribution and use in source and binary forms are permitted
# provided that the above copyright notice and this paragraph are
# duplicated in all such forms and that any documentation, advertising
# materials, and other materials related to such distribution and use
# acknowledge that the software was developed by the University of
# Southern California, Information Sciences Institute.  The name of the
# University may not be used to endorse or promote products derived from
# this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
# 

#
# Maintainer: Kannan Varadhan <kannan@isi.edu>
# Version Date: $Date: 2006/03/08 13:53:00 $
#
# @(#) $Header: /var/lib/cvs/ns-2.29/tcl/ex/srm-adapt-req.tcl,v 1.1.1.1 2006/03/08 13:53:00 rouil Exp $ (USC/ISI)
#
# updated to use -multicast on and Node allocaddr by Lloyd Wood

#
# simple 8 node star topology, runs for 50s, tests Adaptive timers.
#

if [string match {*.tcl} $argv0] {
    set prog [string range $argv0 0 [expr [string length $argv0] - 5]]
} else {
    set prog $argv0
}

if {[llength $argv] > 0} {
    set srmSimType [lindex $argv 0]
} else {
    set srmSimType Adaptive
}

source ../mcast/srm-nam.tcl		;# to separate control messages.
#source ../mcast/srm-debug.tcl		;# to trace delay compute fcn. details.

set ns [new Simulator -multicast on]

$ns trace-all [open out.tr w]
$ns namtrace-all [open out.nam w]

$ns color 0 white		;# data packets
$ns color 40 blue		;# session
$ns color 41 red		;# request
$ns color 42 green		;# repair
$ns color 1 Khaki		;# source node
$ns color 2 goldenrod
$ns color 3 sienna
$ns color 4 HotPink
$ns color 5 maroon
$ns color 6 orchid
$ns color 7 purple
$ns color 8 snow4
$ns color 9 PeachPuff1

# make the nodes
set nmax 8
for {set i 0} {$i <= $nmax} {incr i} {
    set n($i) [$ns node]
}
$n(0) shape "other"
$n(1) shape "box"

# now the links
for {set i 1} {$i <= $nmax} {incr i} {
    $ns duplex-link $n($i) $n(0) 1.5Mb 10ms DropTail
}


$ns duplex-link-op $n(1) $n(0) orient right
$ns duplex-link-op $n(2) $n(0) orient right-down
$ns duplex-link-op $n(3) $n(0) orient up
$ns duplex-link-op $n(4) $n(0) orient left-down
$ns duplex-link-op $n(5) $n(0) orient left
$ns duplex-link-op $n(6) $n(0) orient left-up
$ns duplex-link-op $n(7) $n(0) orient down
$ns duplex-link-op $n(8) $n(0) orient right-up

$ns duplex-link-op $n(0) $n(1) queuePos 0

# configure multicast
set group [Node allocaddr]
set cmc [$ns mrtproto CtrMcast {}]
$ns at 0.3 "$cmc switch-treetype $group"

# now the agents
set srmStats [open srmStats.tr w]
set srmEvents [open srmEvents.tr w]

set fid 0
for {set i 1} {$i <= $nmax} {incr i} {
    set srm($i) [new Agent/SRM/$srmSimType]
    $srm($i) set dst_addr_ $group
    $srm($i) set dst_port_ 0
    $srm($i) set fid_ [incr fid]
    $srm($i) log $srmStats
    $srm($i) trace $srmEvents
    $ns at 1.0 "$srm($i) start"

    $ns attach-agent $n($i) $srm($i)
}

#$srm(4) set C1_ 0.5
#$srm(4) set C2_ 1.0

# And, finally, attach a data source to srm(1)
set packetSize 800
set s [new Application/Traffic/CBR]
$s set packetSize_ $packetSize
$s set interval_ 0.02
$s attach-agent $srm(1)
$srm(1) set tg_ $s
$srm(1) set app_fid_ 0
$srm(1) set packetSize_ $packetSize
$ns at 3.5 "$srm(1) start-source"

# Drop a packet every 0.5 secs. starting at 3.52s.
# Drops packets from the source so all receivers see the loss.

#$ns rtmodel Deterministic {3.021 0.498 0.002} $n(0) $n(1)
set loss_module [new SRMErrorModel]
$loss_module drop-packet 2 200 1
$loss_module drop-target [$ns set nullAgent_]
$ns lossmodel $loss_module $n(1) $n(0)

$ns at 150 "finish $s"

proc finish src {
    $src stop

    global ns srmStats srmEvents srm 
    $ns flush-trace		;# NB>  Did not really close out.tr...:-)
    close $srmStats
    close $srmEvents
    set avg_info [open srm-avg-info w]
    puts $avg_info "avg:\trep-delay\treq-delay\trep-dup\t\treq-dup"
    foreach index [lsort -integer [array name srm]] {
	set tmplist [$srm($index) array get stats_]
	puts $avg_info "$index\t[format %7.4f [lindex $tmplist 1]]\t\t[format %7.4f [lindex $tmplist 5]]\t\t[format %7.4f [lindex $tmplist 9]]\t\t[format %7.4f [lindex $tmplist 13]]"
    }
    flush $avg_info
    close $avg_info

    exit 0
}

set ave [open ave.tr w]
proc doDump {now i tag} {
    global srm ave
    puts $ave [list $now $i					\
	    [$srm($i) set stats_(ave-dup-${tag})]	\
	    [$srm($i) set stats_(ave-${tag}-delay)]]
}
proc dumparams intvl {
    global ns nmax
    set now [$ns now]

    doDump $now 1 rep
    for {set i 2} { $i <= $nmax} {incr i} {
	doDump $now $i req
    }
    $ns at [expr $now + $intvl] "dumparams $intvl"
}
$ns at 4.0 "dumparams 0.5"

$ns run
