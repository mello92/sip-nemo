set ns [new Simulator]

$ns color 1 Red 
$ns color 2 Blue

set nf [open mysip-app-out.nam w]
set tf [open mysip-app-out.tr w]
$ns namtrace-all $nf
$ns trace-all $tf

proc finish {} {
	global ns nf tf
	$ns flush-trace
	close $nf
	close $tf
	exec nam mysip-app-out.nam &
	exit 0
}

set node_(s1) [$ns node]
set node_(s2) [$ns node]
set node_(s3) [$ns node]
set node_(s4) [$ns node]
set node_(r1) [$ns node]
set node_(r2) [$ns node]

$ns duplex-link $node_(s1) $node_(r1) 5Mb 3ms DropTail
$ns duplex-link $node_(s2) $node_(r1) 5Mb 3ms DropTail
$ns duplex-link $node_(r1) $node_(r2) 2Mb 10ms RED
$ns duplex-link $node_(s3) $node_(r2) 5Mb 3ms DropTail
$ns duplex-link $node_(s4) $node_(r2) 5Mb 3ms DropTail

$ns queue-limit $node_(r1) $node_(r2) 20
Queue/RED set thresh_ 5
Queue/RED set maxthresh_ 10
Queue/RED set q_weight_ 0.002
Queue/RED set ave_ 0

$ns duplex-link-op $node_(r1) $node_(r2) queuePos 0.5

$ns duplex-link-op $node_(s1) $node_(r1) orient right-down
$ns duplex-link-op $node_(s2) $node_(r1) orient right-up
$ns duplex-link-op $node_(r1) $node_(r2) orient right
$ns duplex-link-op $node_(s3) $node_(r2) orient left-down
$ns duplex-link-op $node_(s4) $node_(r2) orient left-up

#	Setup a MM UDP connection
set udp_s [new Agent/UDP/Udpmysip]
set udp_r [new Agent/UDP/Udpmysip]
set udp_server [new Agent/UDP/Udpmysip]
$ns attach-agent $node_(s1) $udp_s
$ns attach-agent $node_(s3) $udp_r
$ns attach-agent $node_(r1) $udp_server

$ns connect $udp_s $udp_r 

$udp_s set packetSize_ 1000
$udp_r set packetSize_ 1000
$udp_server set packetSize_ 1000

$udp_s set fid_ 1
$udp_r set fid_ 2


#Setup a MM Application
set mysipapp_s [new Application/mysipApp]
set mysipapp_r [new Application/mysipApp]
set mysipapp_server [new Application/mysipApp]
$mysipapp_s attach-agent $udp_s
$mysipapp_r attach-agent $udp_r
$mysipapp_server attach-agent $udp_server

$mysipapp_s set pktsize_ 1000
$mysipapp_s set random_ false
$mysipapp_s myID_URL 1000 0.0.0
$mysipapp_r myID_URL 9999 4.1.0
$mysipapp_server myID_URL 88 4.1.0
$mysipapp_server add_URL_record 9999  4.1.0 4.1.129
[$node_(r1) set dmux_] install 3 $udp_server

#	Setup a TCP connection
#set tcp [$ns create-connection TCP/Reno $node_(s2) TCPSink $node_(s4) 0]
#$tcp set window_ 15
#$tcp set fid_ 2
#
## Setup a FTP Applicaton
#set ftp [$tcp attach-source FTP]

# Simulation Scenario
#$ns at 0.0 "$ftp start"
#$ns at 1.0 "finish"
#
#set cbr [new Application/Traffic/CBR]
#$cbr attach-agent $udp_s
#$cbr set type_ BCR
#$cbr set packet_size_ 5000
#$cbr set rate_ 1mb
#$cbr set random_ false
#
#$ns at 0.1 "$cbr start"
#$ns at 1.0 "$cbr stop"

#$ns at 0.0 "$ftp start"
#$ns at 1.0 "$mysipapp_s start"

$ns at 0.3 "$mysipapp_r registration"
#$ns at 0.6 "$mysipapp_s registration"
$ns at 1.0 "$mysipapp_s send_invite 9999 4.1.0"
$ns at 3.0 "finish"

$ns run
