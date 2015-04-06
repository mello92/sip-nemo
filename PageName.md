# Introduction #

Add your content here.


# Details #

$ns node-config
> -adhocRouting Network

> -MIPv6 ON/OFF

> -MIPAgent BS/CN/MR/LFN/VMN


---


tcl/lib/ns-lib.tcl
```
Simulator instproc create-wireless-node args {
        switch -exact $routingAgent_ {
        Network {
		    set ragent [$self create-network-agent $node]
	    }
}
```

tcl/nemo/ns-mipv6.tcl
```
Simulator instproc MIPv6 {val} {
	$self instvar mipv6_
	if { $val == "ON" } {
		$self set mipv6_ 1
	} else {
		$self set mipv6_ 0
	}
}

Simulator instproc MIPAgent {val} {
	$self instvar mipagent_
	$self set mipagent_ $val
	#puts "$mipagent_"
}

Simulator instproc create-network-agent { node } {
	set nodetype_ [$self get-nodetype]

	#puts "create-network-agent in ns-mipv6.tcl"

	if { $nodetype_ == "BS" } {
		set ragent [new Agent/NetworkRouting/NetworkBS]
		$ragent mip-agent [$node set regagent_]
	} else {
	if { $nodetype_ == "MN" } {
		set ragent [new Agent/NetworkRouting/NetworkMN]
		$ragent mip-agent [$node set regagent_]
		$ragent decap-port [Simulator set DECAP_PORT]
	} else {
	if { $nodetype_ == "MR" } {
		set ragent [new Agent/NetworkRouting/NetworkMR]
		$ragent mip-agent [$node set regagent_]
		$ragent decap-port [Simulator set DECAP_PORT]
	} else {
	if { $nodetype_ == "LFN" } {
		set ragent [new Agent/NetworkRouting/NetworkLFN]
		$ragent mip-agent [$node set regagent_]
		$ragent decap-port [Simulator set DECAP_PORT]
	} else {
		puts "Wrong Routing Agent"
	}}}}

	set addr [$node node-addr]
	$ragent addr $addr
	$ragent node $node

	if [[Simulator instance] set mipv6_] {
		$ragent port-dmux [$node set dmux_]
	}

	$node set ragent_ $ragent

	return $ragent
}
```

tcl/lib/ns-lib.tcl
```
source ../nemo/ns-mipv6.tcl
source ../nemo/proc-nemo-config.tcl

Simulator instproc get-nodetype {} {
	$self instvar addressType_ routingAgent_ wiredRouting_ mipv6_ mipagent_
        if { [info exists mipv6_] && $mipv6_ == 1 } {
		set val $mipagent_
		#puts "$val"
	}
}
```

---

`set ragent [new Agent/NetworkRouting/NetworkBS]`

`$ragent mip-agent [$node set regagent_]`

---


tcl/lib/ns-node.tcl
```
Node instproc mk-default-classifier {} {
	if [[Simulator instance] set mipv6_] {
		$self makemip-NewHier
	}
}
```

tcl/nemo/ns-mipv6.tcl
```
Simulator set MIPv6_PORT	0

Node instproc makemip-NewHier {} {
	$self instvar regagent_ encap_ decap_ agents_ id_

	#puts "makemip-NewHier in ns-mipv6.tcl"

	set dmux [new Classifier/Port/Reserve]
	$dmux set mask_ 0x7fffffff
	$dmux set shift_ 0
	$self install-demux $dmux
   
	$self attach-redirect

	set mipagent_ [[Simulator instance] set mipagent_]
	set regagent_ [new Agent/$mipagent_ $self]
	$self attach $regagent_ [Simulator set MIPv6_PORT]
}

Node/MobileNode instproc makemip-NewBS {} {
	#puts "makemip-NewBS in ns-mipv6.tcl"
	$self attach-decapsulator
}

Node/MobileNode instproc attach-decapsulator {} {
	$self instvar decap_ dmux_ agents_ regagent_

	#puts "attach-decapsulator in ns-mipv6.tcl"

	set decap_ [new Classifier/Addr/MIPDecapsulator]
	lappend agents_ $decap_

	#set test [$dmux_ info class]
	#puts "$test"
	$dmux_ install [Simulator set DECAP_PORT] $decap_
	$decap_ defaulttarget [$self entry]
}
```

/nemo/ipv6.h
```
class NetworkRouting : public Agent
{
};
class NetworkBS : public NetworkRouting
{
};
```

/nemo/ipv6.cc

/nemo/mipv6.h
```
class MIPv6Agent : public Agent
{
};
class BSAgent : public MIPv6Agent
{
};
```

/nemo/mipv6.cc


| table | cells |
|:------|:------|

Add your content here.  Format your content with:
  * Text in **bold** or _italic_
  * Headings, paragraphs, and lists
  * Automatic links to other wiki pages