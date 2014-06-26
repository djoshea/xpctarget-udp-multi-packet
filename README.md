XPC Target UDP Multi-packet
===========================

This Simulink block serves as a stand-in for the Real-time UDP Send block 
for transmitting UDP data from a Simulink XPC Target real-time engine. It 
accepts a variable-length uint8 array which can be arbitrarily long and sends
this as one or more UDP packets via an internal Ethernet Tx block. In order to
send data larger than 1500 bytes, the block splits the packet using 
[IP fragmentation](http://en.wikipedia.org/wiki/IP_fragmentation). These fragments
will be automatically reassembled on the receiving computer at the internet layer 
of the network stack, so the packets arrive fully reassembled at the client socket.

Mask Parameters and Configuration
---------------------------------

Double clicking on the block will open the block mask where various parameters may be set.
The first parameter is a numeric parameter should be unique across the model, or at least unique among all 
blocks which transmit over the same Ethernet Tx Id to the same IP addresses. The valid range is from 0 to 255, 
i.e. a `uint8`. This parameter will be combined with an incrementing `uint8` counter to form the Identification
field of the [IP header](http://en.wikipedia.org/wiki/IPv4#Packet_structure).

Source and destination IP addresses and ports specify the IP and UDP packet parameters (broadcast IPs okay), 
destination MAC address using the `macaddr` MATLAB function (broadcast MAC `ff:ff:ff:ff:ff:ff` okay), 
and Ethernet Tx Id, which determines which NIC the packet will be sent out of on the XPC target.

There is also a checkbox which toggles the option to prepend a 4-byte header onto each UDP packet, containing the 
length of the data in bytes as a 2-byte uint16, followed by a 2-byte uint16 simple checksum, taken as the sum of all 
bytes in data modulo 2^16. This feature is usually not necessary, as the receiving OS will already validate the 
IP and UDP checksums in the header already, and the length of the packet will also be returned by the socket `read()` 
on the receiving computer, but this is provided as a convenience. Note that the length of the packet returned by `read()`
will be rounded up to the nearest even integer, whereas the length provided by this header may be odd.

![Mask Parameters Screenshot](https://raw.githubusercontent.com/djoshea/xpctarget-udp-multi-packet/master/screenshot.png)

