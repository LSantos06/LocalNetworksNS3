#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include <iostream>
#include <sstream>

/* Requisitos:
 * a) Ao menos duas redes Ethernet (padrão 802.3);
 * b) Quatro redes sem fio (padrão 802.11x);
 * c) Um mínimo de 10 clientes em todas as redes.
 *
 * Os ambientes das redes (a) e (b) devem ser domínios diferentes de colisão.
 * Na rede 802.3 deve existir um servidor de aplicação que precisa ser acessado por nós/clientes das outras redes.
 * A escolha do serviço a ser implementado é livre.
 */

// Topologia:
//
//   Wifi 10.1.8.0
//                 AP
//  *    *    *    *     10.1.2.0
//  |    |....|    |  point-to-point
//  n    n    n   n2 ---------------- n1    n    n    n
//                 :                   |    |....|    |
//                 :                   ================
//     10.1.3.0    :                   : LAN 10.1.7.0
//  point-to-point :                   :
//                 :                   :
//                 :                   :    10.1.1.0
//   Wifi 10.1.9.0 :                   : point-to-point
//                 AP                  :
//  *    *    *    *                   :
//  |    |....|    |                   :
//  n    n    n   n3                  n0    n    n   [n]
//                 :                   |    |....|    |
//                 :                   ================
//     10.1.4.0    :                     LAN 10.1.6.0
//  point-to-point :
//                 :
//                 :      10.1.5.0
//  Wifi 10.1.10.0 :   point-to-point    Wifi 10.1.11.0
//                 AP ---------------- AP
//  *    *    *    *                   *    *    *    *
//  |    |....|    |                   |    |....|    |
//  n    n    n   n4                  n5    n    n    n
//



using namespace ns3;
using namespace std;

int main(int argc,char *argv[]){
	bool verbose = true;
	bool tracing = true;


  /***** ENLACE *****/
  // Number of devices CSMA and WiFi
  uint32_t nCsma = 12;
  uint32_t nWifi = 12;
  /** CMD **/
  // Command line parameters for enabling or disbling components
  CommandLine cmd;
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Disable pcap tracing", tracing);
  cmd.Parse (argc,argv);
  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  /** P2P **/
  // Number of nodes that we will connect via p2p link
	int nP2p = 5;
	NodeContainer p2pNodes[nP2p];

	for(int i=0;i< nP2p-1 ;i++)
		p2pNodes[i].Create(1);
	p2pNodes[4].Create(2);

	for(int i=0;i< nP2p-1 ;i++)
		p2pNodes[i].Add(p2pNodes[i+1].Get(0)); // n_i + n_(i+1)

  // Set the associated default attributes
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  // Installing the devices os the nodes and the channels between them
	NetDeviceContainer p2pDevices[nP2p];
	for(int i=0;i<nP2p;i++)
		p2pDevices[i] = pointToPoint.Install(p2pNodes[i]);


  /** CSMAs **/
  // Hold the nodes that will be part of the bus (CSMA) network
  NodeContainer csmaNodes[2]; // 6 e 7

  // Gets the nodes from the p2p nodes and adds it to the container of nodes that will get CSMA devices
  // The nodes in question are going to end up with a p2p device and a CSMA device
  // We then create a number of "extra" nodes that compose the reaminder of CSMA network

  // We then instantiate a CsmaHelper and set its Attributes
	CsmaHelper csma[2];
	NetDeviceContainer csmaDevices[2];

	for(int i=0;i<2;i++){
		csmaNodes[i].Add(p2pNodes[0].Get(i));
		csmaNodes[i].Create(nCsma);

		csma[i].SetChannelAttribute ("DataRate", StringValue("100Mbps"));
		csma[i].SetChannelAttribute ("Delay", TimeValue(NanoSeconds(6560)));

		csmaDevices[i] = csma[i].Install(csmaNodes[i]);
	}

	//** WIFI **//

	NodeContainer wifiStaNodes[4];
	NodeContainer wifiApNode[4];
	for(int i=0;i<3;i++){
		wifiStaNodes[i].Create(nWifi);
		wifiApNode[i] = p2pNodes[i+2].Get(0);
	}
	wifiStaNodes[3].Create(nWifi);
	wifiApNode[3] = p2pNodes[4].Get(1);

	//Default PHY layer configuration
	YansWifiChannelHelper channel[4];
	YansWifiPhyHelper phy[4];

	WifiHelper wifi[4];

  WifiMacHelper macW, macA;
  // SSID of the infrastructure network
  Ssid ssid = Ssid ("ns-3-ssid");
  // Configuration of the type of MAC
  // Statins don't perform active probing
  macW.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  // Creating wifi devices of these stations
	NetDeviceContainer staDevices[4];

	macA.SetType("ns3::ApWifiMac","Ssid",SsidValue(ssid));
	// Creating Ap Nodes
	NetDeviceContainer apDevices[4];


  // STA nodes to be mobile, wandering around inside a bounding box, and we want to make the AP node stationary
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (sqrt(nWifi)),
                                 "LayoutType", StringValue ("RowFirst"));
	// Installing mobility models on the STA nodes
 	mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                           "Bounds", RectangleValue (Rectangle (-150, 150, -150, 150)));

	for(int i=0;i<4;i++){
		channel[i] = YansWifiChannelHelper::Default();
		phy[i] = YansWifiPhyHelper::Default();

		// Creating a channel object and associating it to our PHY layer
		// All the PHY layer objects share the same wirelles medium and can communicate and interfere
		phy[i].SetChannel(channel[i].Create());
		wifi[i].SetRemoteStationManager("ns3::AarfWifiManager");

		staDevices[i] = wifi[i].Install(phy[i],macW,wifiStaNodes[i]);
		apDevices[i] = wifi[i].Install(phy[i],macA,wifiApNode[i]);

		// Installing mobility models on the STA nodes
		mobility.Install(wifiStaNodes[i]);

		// Installing mobility (fixed position) on the AP nodes
		mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		mobility.Install (wifiApNode[i]); // 8

	}

	/***** REDE *****/
	// Installing protocol stacks
	InternetStackHelper stack;
	for(int i=0;i<2;i++)
		stack.Install(csmaNodes[i]);

	for(int i=0;i<4;i++){
		stack.Install(wifiApNode[i]);
		stack.Install(wifiStaNodes[i]);
	}

	// Assigning IP adresses to our devce interfaces
	Ipv4AddressHelper address;
	Ipv4InterfaceContainer p2pInterfaces[nP2p];
	Ipv4InterfaceContainer csmaInterfaces[2];

	// P2P
	for(int i=0;i<nP2p;i++){
		stringstream ss;
		ss << "10.1." << i+1 << ".0";
		address.SetBase(ss.str().c_str(),"255.255.255.0");
		p2pInterfaces[i] = address.Assign(p2pDevices[i]);
	}

	//CSMA
	for(int i=0;i<2;i++){
		stringstream ss;
		ss << "10.1." << i+6 << ".0";
		address.SetBase(ss.str().c_str(),"255.255.255.0");
		csmaInterfaces[i] = address.Assign(csmaDevices[i]);
	}

	//WIFI
	for(int i=0;i<4;i++){
		stringstream ss;
		ss << "10.1." << i+8 << ".0";
		address.SetBase(ss.str().c_str(),"255.255.255.0");
		address.Assign(staDevices[i]);
		address.Assign(apDevices[i]);
	}

  // Setting up the echo port
  UdpEchoServerHelper echoServer (9);
  // SERVER => The rightmost node
  ApplicationContainer serverApps = echoServer.Install (csmaNodes[0].Get (nCsma));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (70.0));
  // SERVER <=> CLIENT
  UdpEchoClientHelper echoClient (csmaInterfaces[0].GetAddress (nCsma), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
  // CLIENTS => 10 for each network
  ApplicationContainer clientApps = echoClient.Install (wifiStaNodes[0]);
	for(int i=1;i<4;i++)
		clientApps.Add(echoClient.Install (wifiStaNodes[i]));
	for(int i=0;i<2;i++)
		clientApps.Add(echoClient.Install (csmaNodes[i]));

  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (70.0));
  // Enabling internetwork
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  if (tracing == true){
      pointToPoint.EnablePcapAll ("proj_2_tr1");
			for(int i=0;i<4;i++){
				stringstream ss;
				ss << "proj_2_tr1_" << i+8;
				phy[i].EnablePcap (ss.str(), apDevices[i].Get (0));
			}
			for(int i=0;i<2;i++){
				stringstream ss;
				ss << "proj_2_tr1_" << i+6;
				csma[i].EnablePcap (ss.str(), csmaDevices[i].Get (0), true);
			}
	}

  // Install FlowMonitor on all nodes
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  NS_LOG_UNCOND(":::::::::: SIMULATION ::::::::::\n");
  // Run simulation for 70 seconds
  Simulator::Stop (Seconds (70));
  Simulator::Run ();

  // Print per flow statistics in xml file
  flowmon.SerializeToXmlFile("statistics.xml", true, true);

  // Print per flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

  NS_LOG_UNCOND("\n:::::::::: STATISTICS ::::::::::");
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter){
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
    NS_LOG_UNCOND("\nFlow ID (" << iter->first << ") " << t.sourceAddress << " => " << t.destinationAddress);
    NS_LOG_UNCOND("Tx Packets = " << iter->second.txPackets);
    NS_LOG_UNCOND("Tx Bytes = " << iter->second.txBytes);
    NS_LOG_UNCOND("Tx offered = " << iter->second.txBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024 / 1024 << " Mbps");
    NS_LOG_UNCOND("Rx Packets = " << iter->second.rxPackets);
    NS_LOG_UNCOND("Rx Packets = " << iter->second.rxBytes);
    NS_LOG_UNCOND("Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024 / 1024 << " Mbps");
  }

  // End simulation
  Simulator::Destroy ();

  return 0;
}
