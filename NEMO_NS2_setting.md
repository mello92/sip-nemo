# MIPv6-NEMO #
mysip-mr.tcl 3 1


---


Global

Agent/MIHUser/IFMNGMT/MIPV6 set exp`_` 0	;(1,1,1), (1,1,n), (1,n,1)

Agent/MIHUser/IFMNGMT/MIPV6 set exp`_` 4	;(1,n,n)

CN
  1. create node by addr
  1. set link
  1. install-default-ifmanager (mipv6)
  1. mipv6 set-node-type CN
  1. ~~create udp~~agent~~
  1. _create cbr_ agent~~
  1. ~~cbr_attach-agent udp_~~
  1. create udpmysipagent udp\_s
  1. attach udp\_s on port 3
  1. udp\_s set-mipv6
  1. create mysipapp agent mysipapp\_s
  1. mysipapp\_s attach-agent udp\_s
  1. mysipapp\_s  set pktsize_, random_, myID\_URL
  1. at time "mysipapp\_s send\_invite URL IP"

MN\_HA
  1. create node by addr
  1. set link
  1. install-default-ifmanager (mipv6)
  1. mipv6 set-node-type MN\_HA
  1. create udpmysipagent udp\_server
  1. attach udp\_server on port 3
  1. create mysipapp agent mysipapp\_server
  1. mysipapp\_server attach-agent udp\_server
  1. mysipapp\_server set myID\_URL, add\_URL\_record

MR\_HA
  1. create node by addr
  1. set link
  1. install-default-ifmanager (mipv6)
  1. mipv6 set-node-type  MR\_HA

BS
  1. create node by addr
  1. set node x, y, z
  1. set mac bss\_id, enable-beacon, channel
  1. set link to backbone
  1. install-default-ifmanager (mipv6)
  1. install-mih
  1. mipv6 connect-mih
  1. mac set mih
  1. mih add-mac

MR
  1. create multiface-node by addr
  1. create interface-node by addr
  1. set interface-node base-station, x, y, z
  1. create nemo-node by addr (BS)
  1. set nemo-node x, y, z
  1. set nemo-node mac bss\_id, enable-beacon, channel
  1. multiface-node add interface node
  1. interface-node install-nd
  1. nemo-node  install-nd
  1. nemo-node-nd set-router, router-lifetime
  1. nemo-node-nd start-ra
  1. create handover
  1. multiface-node install-ifmanager handover
  1. interface-node-nd set-ifmanager handover
  1. multiface-node install-mih
  1. handover connect-mih mih
  1. handover nd\_mac interface-node-mac
  1. multiface-node install-nemo 200~203
  1. multiface-node-nemo connect-interface interface-node (4 interface-node)
  1. ~~create null agent~~
  1. ~~multiface-node attach-agnet null interface-node 4~~
  1. ~~handover add-flow null udp interface-node 1~~
  1. create-dch interface-node null
  1. attach-dch interface-node handover dch
  1. interface-node-mac mih mih (3 iface-node)
  1. handover add-flow udp\_r udp\_s interface-node 2
  1. handover set-mr
```
#(1,1,n)
$handover set-mr 8.0.0 8.0.1 6.0.0 $nemo_mr_eface1 $nemo_mr_iface1
$handover set-mr 8.0.0 8.0.2 11.0.0 $nemo_mr_eface2 $nemo_mr_iface1

#(1,n,1)
$handover set-mr 8.0.0 8.0.1 6.0.0 $nemo_mr_eface1 $nemo_mr_iface1
$handover set-mr 9.0.0 8.0.2 11.0.0 $nemo_mr_eface2 $nemo_mr_iface1

#(1,n,n)
$handover set-mr 8.0.0 8.0.1 6.0.0 $nemo_mr_eface1 $nemo_mr_iface1
$handover set-mr 9.0.0 8.0.2 11.0.0 $nemo_mr_eface2 $nemo_mr_iface1
```
  1. handover set-node-type MR

MN
  1. create multiface-node by addr
  1. create interface-node by addr
  1. set interface-node base-station, x, y, z
  1. multiface-node add interface node
  1. interface-node install-nd
  1. multiface-node install-nemo 200
  1. multiface-node-nemo connect-interface interface-node
  1. install-default-ifmanager (mipv6)
  1. interface-node-nd set-ifmanager mipv6
  1. mipv6 set-node-type MN
  1. create udpmysipagent udp\_r
  1. udp\_r set-mipv6
  1. multiface-node attach-agnet udp\_r interface-node 3
  1. handover add-flow udp\_r udp\_s interface-node
  1. create mysipapp agent mysipapp\_r
  1. mysipapp\_r attach-agent udp\_r
  1. mysipapp\_r myID\_URL
  1. mysipapp\_r log\_file
  1. at time "mysipapp\_r dump\_handoff\_info"


# SIP-NEMO #
mysip-mr3.tcl 3 1


---


Global

Agent/MIHUser/IFMNGMT/MIPv6/Handover/Handover1 set confidence\_th`_` 100	; disable 802\_21

#Agent/MIHUser/IFMNGMT/MIPv6/Handover/Handover1 set confidence\_th`_` 100	; enable 802\_21

CN
  1. create node by addr
  1. set link
  1. install-default-ifmanager (mipv6)
  1. mipv6 set-node-type CN
  1. ~~create udp~~agent~~
  1. _create cbr_ agent~~
  1. ~~cbr_attach-agent udp_~~
  1. create udpmysipagent udp\_s
  1. **udp\_s set-node-type SIP\_CN**
  1. attach udp\_s on port 3
  1. **~~udp\_s set-mipv6~~**
  1. **mipv6 set-udpmysip udp\_s**
  1. create mysipapp agent mysipapp\_s
  1. mysipapp\_s attach-agent udp\_s
  1. mysipapp\_s  set pktsize_, random_, myID\_URL
  1. at time "mysipapp\_s send\_invite URL IP"

MN\_HA
  1. create node by addr
  1. set link
  1. install-default-ifmanager (mipv6)
  1. mipv6 set-node-type MN\_HA
  1. create udpmysipagent udp\_server
  1. **udp\_server set-node-type SIP\_MN\_HA**
  1. attach udp\_server on port 3
  1. create mysipapp agent mysipapp\_server
  1. mysipapp\_server attach-agent udp\_server
  1. mysipapp\_server set myID\_URL, add\_URL\_record

MR\_HA
  1. create node by addr
  1. set link
  1. install-default-ifmanager (mipv6)
  1. mipv6 set-node-type  MR\_HA
  1. **create udpmysipagent udp\_mr\_ha**
  1. **udp\_mr\_ha set-node-type SIP\_MR\_HA**
  1. **attach udp\_mr\_ha on port 3**

BS
  1. create node by addr
  1. set node x, y, z
  1. set mac bss\_id, enable-beacon, channel
  1. set link to backbone
  1. install-default-ifmanager (mipv6)
  1. install-mih
  1. mipv6 connect-mih
  1. mac set mih
  1. mih add-mac

MR
  1. create multiface-node by addr
  1. create interface-node by addr
  1. set interface-node base-station, x, y, z
  1. create nemo-node by addr (BS)
  1. set nemo-node x, y, z
  1. set nemo-node mac bss\_id, enable-beacon, channel
  1. multiface-node add interface node
  1. interface-node install-nd
  1. nemo-node  install-nd
  1. nemo-node-nd set-router, router-lifetime
  1. nemo-node-nd start-ra
  1. create handover
  1. multiface-node install-ifmanager handover
  1. interface-node-nd set-ifmanager handover
  1. multiface-node install-mih
  1. handover connect-mih mih
  1. handover nd\_mac interface-node-mac
  1. multiface-node install-nemo 200~203
  1. multiface-node-nemo connect-interface interface-node (4 interface-node)
  1. ~~create null agent~~
  1. ~~multiface-node attach-agnet null interface-node 4~~
  1. ~~handover add-flow null udp interface-node 1~~
  1. create-dch interface-node null
  1. attach-dch interface-node handover dch
  1. interface-node-mac mih mih (3 iface-node)
  1. **create udpmysipagent udp\_mr**
  1. **udp\_mr set-node-type SIP\_MR**
  1. **udp\_mr set-sip-mr**
  1. **attach udp\_mr on port 3**
  1. **handover set-udpmysip udp\_mr**
  1. **multiface-node attach-agnet udp\_mr interface-node 3**
  1. handover add-flow udp\_r udp\_s interface-node 2
  1. **~~handover set-mr~~**
  1. handover set-node-type MR

MN
  1. create multiface-node by addr
  1. create interface-node by addr
  1. set interface-node base-station, x, y, z
  1. multiface-node add interface node
  1. interface-node install-nd
  1. multiface-node install-nemo 200
  1. multiface-node-nemo connect-interface interface-node
  1. install-default-ifmanager (mipv6)
  1. interface-node-nd set-ifmanager mipv6
  1. mipv6 set-node-type MN
  1. create udpmysipagent udp\_r
  1. **~~udp\_r set-mipv6~~**
  1. **udp\_r set-node-type SIP\_MN**
  1. **udp\_r set-sip-mn**
  1. **attach udp\_r on port 3**
  1. **mipv6 set-udpmysip udp\_r**
  1. multiface-node attach-agnet udp\_r interface-node 3
  1. handover add-flow udp\_r udp\_s interface-node
  1. create mysipapp agent mysipapp\_r
  1. mysipapp\_r attach-agent udp\_r
  1. mysipapp\_r myID\_URL
  1. mysipapp\_r log\_file
  1. at time "mysipapp\_r dump\_handoff\_info"


# Details #

Add your content here.  Format your content with:
  * Text in **bold** or _italic_
  * Headings, paragraphs, and lists
  * Automatic links to other wiki pages