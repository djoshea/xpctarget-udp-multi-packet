Multi-Packet UDP for xPC Target
===============================

This tool provides a Simulink block which automatically splits data (as a vector of `uint8`'s)
across as many UDP packets as necessary. 

![Block screenshot](https://raw.github.com/djoshea/xpctarget-udp-multi-packet/master/blockScreenshot.png)

Each packet is prefixed with a short (~ 20 byte) header 
which identifies the packets as arising from this block. This prefix also carries information about:

* Which block id sent the packet (in case multiple instances of the multi packet block are directed at the same ethernet card / port.
* How many packets were sent in total
* Which packet index this current packet was

In order to use this block, you will need to provide an appropriate timestamp (of type double) which 
uniquely identifies each xPC target tick and store the timestamp in the Data Store Memory `MultiPacketTimestamp`. The
value of this timestamp isn't important, only that it be unique for each tick, as it is used to group the packets together
at the receiving end. A monotonic free-running counter would serve this purpose well.

You will also need to set the send and receive IP addresses and ports, transmit block id, and Ethernet Tx Id. 
**Make sure that the transmit block id is unique across the model!!!**
These parameters are all accessible via the block's mask; double click to set these.

In `src`, there is multithreaded C code for building a ring buffer of Packets and PacketSets (groups of packets containing pieces of the same data block)
which handle reassembling the packets at the receiving Linux PC. **You will need to provide additional "raw data has arrived" where indicated inside `parser.c`.**

