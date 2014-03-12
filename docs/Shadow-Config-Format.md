Shadow uses a standard XML format to accept configuration options from users, and uses [GLib's XML parser](https://developer.gnome.org/glib/stable/glib-Simple-XML-Subset-Parser.html) to parse the simple structure. The following are valid elements and their attributes for a shadow configuration file (`shadow.config.xml`):

### The _cdf_ element
```xml
<cdf id="STRING" path="STRING" center="INTEGER" width="INTEGER" tail="INTEGER" />
```
**Required attributes**: _id_, _path_ or _center_  
**Optional attributes**: _width_, _tail_

The _cdf_ element instructs Shadow to either generate an empirical Cumulative Distribution Function, or load the representation of one from a file. The _id_ attribute identifies this _cdf_ and must be a string that is unique among all _id_ attributes for any element in the XML file.

If no _path_ is given, it will generate a CDF using _center_ - _width_ as the 10th percentile, _center_ as the 80th percentile, _center_ + _width_ as the 90th percentile, and _center_ + _width_ + _tail_ as the 95th percentile.

If _path_ is given, it should specify the location of a file from which Shadow should extract a CDF. If _path_ begins with `~/`, the path will be considered relative to the current user's home directory. The file should be in the following format:

```text
1000.000 0.2000000000
2000.000 0.4000000000
3000.000 0.6000000000
4000.000 0.8000000000
5000.000 1.0000000000
```

Where the first column represents the value, and the second represents the percentile.

### The _cluster_ element
```xml
<cluster id="STRING" bandwidthdown="INTEGER" bandwidthup="INTEGER" packetloss="FLOAT" />
```
**Required attributes**: _id_, _bandwidthdown_, _bandwidthup_  
**Optional attribute**: _packetloss_

The _cluster_ element represents a vertex in the network topology, i.e., the entity to which we will attach virtual hosts. The _id_ attribute identifies this _cluster_ and must be a string that is unique among all _id_ attributes for any element in the XML file. 

_bandwidthdown_ and _bandwidthup_ represent the default downstream and upstream bandwidth capacities for hosts attached to this cluster. The default values are used for _nodes_ that do not supply their own bandwidth overrides. 

The _packetloss_ attribute is optional and represents the percentage chance that any given packet traveling through this cluster will be lost (independent of _link_ _packetloss_ rates, i.e., there are 3 end-to-end chances to drop a packet).

### The _kill_ element
```xml
<kill time="INTEGER" />
```
**Required attribute**: _time_  

The _time_ attribute represents the number of virtual seconds to simulate, after which the experiment will be killed and resources released.

### The _link_ element
```xml
<link clusters="STRING STRING" latency="INTEGER" jitter="INTEGER" packetloss="FLOAT" />
```
**Required attributes**: _clusters_, _latency_  
**Optional attributes**: _id_, _jitter_, _packetloss_

The _link_ element represents an edge in the network topology, and is used to connect two _clusters_. The _id_ attribute identifies this _link_ and must be a string that is unique among all _id_ attributes for any element in the XML file.

The _clusters_ attribute is a string that specifies the _ids_ of the two clusters the link is connecting, separated by a space. So, to connect `<cluster id="c1" ...` and `<cluster id="c2" ...`, you would set `clusters="c1 c2"`. 

The packet delay across this _link_ in is specified with the _latency_ attribute, and the average variation in packet delay is specified with the _jitter_ attribute. Both _latency_ and _jitter_ are specified in milliseconds. 

The _packetloss_ attribute is optional and represents the percentage chance that any given packet traveling through this link will be lost (independent of _cluster_ _packetloss_ rates, i.e., there are 3 end-to-end chances to drop a packet).

### The _node_ element
```xml
<node id="STRING" cluster="STRING" quantity="INTEGER" bandwidthdown="INTEGER" bandwidthup="INTEGER" interfacebuffer="INTEGER" socketrecvbuffer="INTEGER" socketsendbuffer="INTEGER" loglevel="STRING" heartbeatloglevel="STRING" heartbeatfrequency="INTEGER" cpufrequency="INTEGER" logpcap="STRING" pcapdir="STRING">
  <application ... />
  ...
</node>
```
**Required attributes**: _id_  
**Optional attributes**: _cluster_, _quantity_, _bandwidthdown_, _bandwidthup_, _interfacebuffer_, _socketrecvbuffer_, _socketsendbuffer_, _loglevel_, _heartbeatloglevel_, _heartbeatfrequency_, _cpufrequency_, _logpcap_, _pcapdir_  
**Required child element**: \<application\>  

The _node_ element represents a node or virtual host in the simulation. The _id_ attribute identifies this _node_ and must be a string that is unique among all _id_ attributes for any element in the XML file. _id_ will also be used as the network hostname of this _node_.

The _cluster_ attribute optionally specifies to which network vertex this _node_ should be assigned. If not given, a random vertex will be chosen internally. The _quantity_ attribute specifies the number of hosts of this type to start. If _quantity_ is greater than 1, each host's hostname will be prefixed with a counter. For example, a _node_ with an _id_ of `host` and _quantity_=2 would produce nodes with hostnames `1.host` and `2.host`.

_bandwidthdown_ and _bandwidthup_ optionally specify the downstream and upstream bandwidth capacities for this _node_, and override any default bandwidth values set in the _cluster_ element corresponding to the _cluster_ attribute. If not given, the default bandwidth values from the assigned _cluster_ element are used.

_interfacebuffer_ controls the size of the interface receive buffer that accepts packets from the network. _socketrecvbuffer_ and _socketsendbuffer_ control the initial size of the socket buffers that hold packets to and from the application. Note that these sizes may be adjusted by auto-tuning, in order fill the channel capacity as defined by the bandwidth-delay product between two nodes. These values can instead be set globally for all nodes with the Shadow command line options `--interface-buffer`, `--socket-recv-buffer`, and `--socket-send-buffer` (see `shadow --help-network` for more info).

_loglevel_ and _heartbeatloglovel_ are node-specific overrides for the simulator default log levels (the defaults are adjustable with shadow arguments `--log-level` and `--heartbeat-log-level`). Valid strings include 'error', 'critical', 'warning', 'message', 'info', and 'debug'. _heartbeatfrequency_ is a node-specific override for the default number of seconds between which heartbeat messages are logged (the default is adjustable with shadow argument `--heartbeat-frequency`). Each heartbeat message contains useful statistics about the _node_.

_cpufrequency_ is the speed of this _node's_ virtual CPU in kilohertz. Along with the CPU processing requirements of the plug-in application, this determines how often events for this _node_ are delayed during simulation.

_logpcap_ is a case insenstive boolean string (e.g. "true") that specifies that Shadow should log all network input and output for this _node_ in PCAP format (for viewing in e.g. wireshark). _pcapdir_ is the directory to which the logs should be saved for this _node_.

Nodes must have at least one child \<application\> (see below), and may have more than one.

### The _plugin_ element
```xml
<plugin id="STRING" path="STRING" />
```
**Required attributes**: _id_, _path_  

The _plugin_ element represents a library plug-in that Shadow should dynamically load. The _id_ attribute identifies this _plugin_ and must be a string that is unique among all _id_ attributes for any element in the XML file. 

The _path_ attribute holds the system path to the plug-in `*.so` library. If _path_ begins with `~/`, the path will be considered relative to the current user's home directory.

### The _application_ element
```xml
<application plugin="STRING" starttime="INTEGER" stoptime="INTEGER" arguments="STRING" />
```
**Required attributes**: _plugin_, _starttime_, _arguments_  
**Optional attributes**: _stoptime_  
**Required parent element**: \<node\>

The _application_ element represents an application the node will run. The _plugin_ attribute should be set to the _id_ of the _plugin_ element that represents the plug-in that should be used to launch this application at _starttime_ virtual seconds from the beginning of the simulation. The application will be stopped at _stoptime_ virtual seconds if given.

The _arguments_ attribute should be set to a string holding the required plug-in arguments. This string will be passed to the plug-in in an `argv`-style array, similar to how arguments are passed to the main function in a `C` program. Please see the plug-in documentation for usage and format of the argument string.