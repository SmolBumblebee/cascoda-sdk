# Getting Started with Thread

This document aims to introduce the Thread Protocol, and how to form and use a Thread Network using Cascoda hardware.

This document assumes the use of the [Cascoda Thread Evaluation Kit.](https://www.cascoda.com/wheretobuy/)

![Thread Certified Component Logo](../../etc/img/thread_certified_component.png)

## Contents

<!-- TOC -->

- [Getting Started with Thread](#getting-started-with-thread)
	- [Contents](#contents)
	- [The System Overview](#the-system-overview)
	- [Step 1 - Get the binaries](#step-1---get-the-binaries)
	- [Step 2 - Access the Border Router](#step-2---access-the-border-router)
	- [Step 3 - Form the Network](#step-3---form-the-network)
	- [Step 4 - Join the other devices to the network](#step-4---join-the-other-devices-to-the-network)
	- [Step 5 - Demonstrate communication](#step-5---demonstrate-communication)

<!-- /TOC -->

## The System Overview

The [Cascoda Thread Evaluation Kit](https://www.cascoda.com/wheretobuy/) contains most of the required parts for evaluating Thread functionality. In addition, a Cascoda Border Router is required.

The Chili2D USB dongles are used as Thread devices, which in a real network could be sensors or actuators. 
The Thread stack is running on the Chili2 in the [ot-cli](../../baremetal/cascoda-bm-thread/example) firmware, and can be controlled using the 'OpenThread CLI' which is available over USB.
This is the [On-Module](../reference/system-architecture.md#on-module) system model.
This firmware has been configured to use USB as a serial interface.
To access the OpenThread CLI, [serial-adapter](../../posix/app/serial-adapter) should be run on a host operating system which the Chili2D is connected to via USB.

Within the border router, a Chili2S is used as an IEEE 802.15.4 interface, running [mac-dongle](../../baremetal/app/mac-dongle) as firmware. The Thread
stack is running on the router's processor, taking advantage of the additional resources compared to the micro-controller. 
This is the [Radio Co-Processor](../reference/system-architecture.md#radio-co-processor) system model.
The Border Router is capable of routing IPv6 traffic between the Thread Network and the adjacent
Ethernet network and hosted WiFi access point. It also hosts a Web UI to allow control of the Thread Network.

<p align="center"><img src="puml/thread/png/mesh.png" width="50%" align="center"></p>

*Example Mesh network for the 4-Chili evaluation kit, with 2 router enabled nodes, and 2 end device nodes. Note
that the end device nodes may connect to any Router device.*

## Step 1 - Get the binaries

The binaries should be pre-flashed on the devices when the kit is received, but in case it is an old version, or if the firmware has
been overwritten, it can be reflashed.

Pre-built binaries can be obtained from the [GitHub releases page](https://github.com/Cascoda/cascoda-sdk/releases), or binaries can be
built from scratch by following the [build instructions.](../../README.md#building) It is important that the USB modules are flashed with firmware configured to have a [USB interface](../reference/cmake-configuration.md#cascoda_bm_interface).

The Chili2D USB devices should be flashed with [ot-cli](../../baremetal/cascoda-bm-thread/example).

To flash the firmware over USB (or other), consult the [flashing guide.](../../docs/guides/flashing.md)

## Step 2 - Access the Border Router

The Border Router Web GUI is accessible via Ethernet or WiFi, though by default the WiFi Access Point is disabled. The easiest way to access the GUI is by connecting a PC to the LAN1 Ethernet port and navigating to [http://openwrt.local](http://openwrt.local). You will then be prompted to log in. By default, there is no password - hitting "Login" at the "Authorisation Required" screen will grant you access.

Note that the Web GUI is only accessible via the Ethernet port marked "PoE LAN1". If you need to access it from "WAN / LAN2", you must create a rule in the firewall that allows access to the router from that port - this is outside the scope of this guide. 

For more information about the border router, see the [Setup Guide.](../guides/border-router-setup.md)

## Step 3 - Form the Network

Form a network by following the [network formation guide.](../guides/thread-network-formation.md)

## Step 4 - Join the other devices to the network

After forming a network, add the Chili2D devices by following [the commissioning process.](../guides/thread-commissioning.md)

## Step 5 - Demonstrate communication

To demonstrate communication between the devices, the simplest mechanism is to try to send a ping between the devices.

Using ``serial-adapter`` to control one of the ``ot-cli`` devices, get the 'mesh local' IPv6 address using the ``ipaddr`` command.

```bash
> ipaddr
fd3d:b50b:f96d:722d:0:ff:fe00:fc00
fd3d:b50b:f96d:722d:0:ff:fe00:c00
fd3d:b50b:f96d:722d:7a73:bff6:9093:9117
fe80:0:0:0:6c41:9001:f3d6:4148
Done
```
In this case, the first is the 'leader anycast' address, and be identified by it's  ``:0:ff:fe00:fc00`` suffix. It will always be owned
by the current leader of the Thread Network partition. 
The ``fd3d:b50b:f96d:722d:0:ff:fe00:c00``  address is the 'RLOC' address, and can change if the network topology changes.
The ``fd3d:b50b:f96d:722d:7a73:bff6:9093:9117`` address is the 'Mesh Local' address, and will not change, but cannot be routed off-mesh.
Note that those first 3 addresses all have the same prefix, which is based off the Extended PAN ID of the network.
The ``fe80::`` prefixed address is link local, so that is usually not useful as it only works over one hop.

If the border router has been configured correctly, there will also be a global IPv6 address in that list, that can be routed off mesh. 

So from the other device, ping the mesh-local address:

```bash
> ping fd3d:b50b:f96d:722d:7a73:bff6:9093:9117
16 bytes from fd3d:b50b:f96d:722d:7a73:bff6:9093:9117: icmp_seq=1 hlim=64 time=24ms
```

You should also be able to ping from the border router. 

In order to determine the topology of the network, the ``neighbor table`` command can be used on the CLI. To allow a device
to become a router (or force it to become a child), the ``routerrole enable`` and ``routerrole disable`` commands can be used.
For more information on commands, see the [OpenThread CLI Reference.](https://github.com/Cascoda/openthread/blob/ext-mac-dev/src/cli/README.md)
