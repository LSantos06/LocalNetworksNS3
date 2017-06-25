#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"

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

 NS_LOG_COMPONENT_DEFINE ("Projeto_2_TR1");

int main (int argc, char *argv[]){
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
  NodeContainer p2pNodes1;
  NodeContainer p2pNodes2;
  NodeContainer p2pNodes3;
  NodeContainer p2pNodes4;
  NodeContainer p2pNodes5;
  p2pNodes1.Create (1); // n0
  p2pNodes2.Create (1); // n1
  p2pNodes3.Create (1); // n2
  p2pNodes4.Create (1); // n3
  p2pNodes5.Create (2); // n4, n5
  p2pNodes1.Add (p2pNodes2.Get (0)); // n0 + n1
  p2pNodes2.Add (p2pNodes3.Get (0)); // n1 + n2
  p2pNodes3.Add (p2pNodes4.Get (0)); // n2 + n3
  p2pNodes4.Add (p2pNodes5.Get (0)); // n3 + n4
  // Set the associated default attributes
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  // Installing the devices os the nodes and the channels between them
  NetDeviceContainer p2pDevices1;
  NetDeviceContainer p2pDevices2;
  NetDeviceContainer p2pDevices3;
  NetDeviceContainer p2pDevices4;
  NetDeviceContainer p2pDevices5;
  p2pDevices1 = pointToPoint.Install (p2pNodes1);
  p2pDevices2 = pointToPoint.Install (p2pNodes2);
  p2pDevices3 = pointToPoint.Install (p2pNodes3);
  p2pDevices4 = pointToPoint.Install (p2pNodes4);
  p2pDevices5 = pointToPoint.Install (p2pNodes5);

  /** CSMAs **/
  // Hold the nodes that will be part of the bus (CSMA) network
  NodeContainer csmaNodes6; // 6

  NodeContainer csmaNodes7; // 7

  // Gets the nodes from the p2p nodes and adds it to the container of nodes that will get CSMA devices
  // The nodes in question are going to end up with a p2p device and a CSMA device
  // We then create a number of "extra" nodes that compose the reaminder of CSMA network
  csmaNodes6.Add (p2pNodes1.Get (0)); //n0
  csmaNodes6.Create (nCsma); // 6

  csmaNodes7.Add (p2pNodes1.Get (1)); //n1
  csmaNodes7.Create (nCsma); // 7

  // We then instantiate a CsmaHelper and set its Attributes
  CsmaHelper csma6;
  csma6.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma6.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560))); // 6

  CsmaHelper csma7;
  csma7.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma7.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560))); // 7

  // We create a NetDeviceContainer to keep track of the created CSMA net devices and then we Install CSMA devices on the selected nodes
  NetDeviceContainer csmaDevices6;
  csmaDevices6 = csma6.Install (csmaNodes6); // 6

  NetDeviceContainer csmaDevices7;
  csmaDevices7 = csma7.Install (csmaNodes7); // 7

  /** WIFI **/
  // Creating the nodes that will be part of the Wifi network
  NodeContainer wifiStaNodes8;
  wifiStaNodes8.Create (nWifi);
  NodeContainer wifiApNode8 = p2pNodes3.Get (0); //n2 // 8

  NodeContainer wifiStaNodes9;
  wifiStaNodes9.Create (nWifi);
  NodeContainer wifiApNode9 = p2pNodes4.Get (0); //n3 // 9

  NodeContainer wifiStaNodes10;
  wifiStaNodes10.Create (nWifi);
  NodeContainer wifiApNode10 = p2pNodes5.Get (0); //n4 // 10

  NodeContainer wifiStaNodes11;
  wifiStaNodes11.Create (nWifi);
  NodeContainer wifiApNode11 = p2pNodes5.Get (1); //n5 // 11

  // Default PHY layer configuration
  YansWifiChannelHelper channel8 = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy8 = YansWifiPhyHelper::Default (); // 8

  YansWifiChannelHelper channel9 = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy9 = YansWifiPhyHelper::Default (); // 9

  YansWifiChannelHelper channel10 = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy10 = YansWifiPhyHelper::Default (); // 10

  YansWifiChannelHelper channel11 = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy11 = YansWifiPhyHelper::Default (); // 11

  // Creating a channel object and associating it to our PHY layer
  // All the PHY layer objects share the same wirelles medium and can communicate and interfere
  phy8.SetChannel (channel8.Create ()); // 8

  phy9.SetChannel (channel9.Create ()); // 9

  phy10.SetChannel (channel10.Create ()); // 10

  phy11.SetChannel (channel11.Create ()); // 11

  // MAC layer
  WifiHelper wifi8;
  wifi8.SetRemoteStationManager ("ns3::AarfWifiManager"); // 8

  WifiHelper wifi9;
  wifi9.SetRemoteStationManager ("ns3::AarfWifiManager"); // 9

  WifiHelper wifi10;
  wifi10.SetRemoteStationManager ("ns3::AarfWifiManager"); // 10

  WifiHelper wifi11;
  wifi11.SetRemoteStationManager ("ns3::AarfWifiManager"); // 11

  WifiMacHelper mac;
  // SSID of the infrastructure network
  Ssid ssid = Ssid ("ns-3-ssid");
  // Configuration of the type of MAC
  // Statins don't perform active probing
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));
  // Creating wifi devices of these stations
  NetDeviceContainer staDevices8;
  staDevices8 = wifi8.Install (phy8, mac, wifiStaNodes8); // 8

  NetDeviceContainer staDevices9;
  staDevices9 = wifi9.Install (phy9, mac, wifiStaNodes9); // 9

  NetDeviceContainer staDevices10;
  staDevices10 = wifi10.Install (phy10, mac, wifiStaNodes10); // 10

  NetDeviceContainer staDevices11;
  staDevices11 = wifi11.Install (phy11, mac, wifiStaNodes11); // 11

  // Configuring access point node
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));
  // Creating AP nodes
  NetDeviceContainer apDevices8;
  apDevices8 = wifi8.Install (phy8, mac, wifiApNode8); // 8

  NetDeviceContainer apDevices9;
  apDevices9 = wifi9.Install (phy9, mac, wifiApNode9); // 9

  NetDeviceContainer apDevices10;
  apDevices10 = wifi10.Install (phy10, mac, wifiApNode10); // 10

  NetDeviceContainer apDevices11;
  apDevices11 = wifi11.Install (phy11, mac, wifiApNode11); // 11

  // STA nodes to be mobile, wandering around inside a bounding box, and we want to make the AP node stationary
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (sqrt(nWifi)),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-150, 150, -150, 150)));
  // Installing mobility models on the STA nodes
  mobility.Install (wifiStaNodes8); // 8

  mobility.Install (wifiStaNodes9); // 9

  mobility.Install (wifiStaNodes10); // 10

  mobility.Install (wifiStaNodes11); // 11

  // Installing mobility (fixed position) on the AP nodes
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode8); // 8

  mobility.Install (wifiApNode9); // 9

  mobility.Install (wifiApNode10); // 10

  mobility.Install (wifiApNode11); // 11

  /***** REDE *****/
  // Installing protocol stacks
  InternetStackHelper stack;
  stack.Install (csmaNodes6); // 6
  stack.Install (csmaNodes7); // 7

  stack.Install (wifiApNode8); // 8
  stack.Install (wifiApNode9); // 9
  stack.Install (wifiApNode10); // 10
  stack.Install (wifiApNode11); // 11

  stack.Install (wifiStaNodes8); // 8
  stack.Install (wifiStaNodes9); // 9
  stack.Install (wifiStaNodes10); // 10
  stack.Install (wifiStaNodes11); // 11

  // Assigning IP adresses to our device interfaces
  Ipv4AddressHelper address;
  // P2P
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces1;
  p2pInterfaces1 = address.Assign (p2pDevices1); // 1

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces2;
  p2pInterfaces2 = address.Assign (p2pDevices2); // 2

  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces3;
  p2pInterfaces3 = address.Assign (p2pDevices3); // 3

  address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces4;
  p2pInterfaces4 = address.Assign (p2pDevices4); // 4

  address.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces5;
  p2pInterfaces5 = address.Assign (p2pDevices5); // 5

  // CSMA
  address.SetBase ("10.1.6.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces6;
  csmaInterfaces6 = address.Assign (csmaDevices6); // 6

  address.SetBase ("10.1.7.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces7;
  csmaInterfaces7 = address.Assign (csmaDevices7); // 7

  // WIFI
  address.SetBase ("10.1.8.0", "255.255.255.0");
  address.Assign (staDevices8);
  address.Assign (apDevices8); // 8

  address.SetBase ("10.1.9.0", "255.255.255.0");
  address.Assign (staDevices9);
  address.Assign (apDevices9); // 9

  address.SetBase ("10.1.10.0", "255.255.255.0");
  address.Assign (staDevices10);
  address.Assign (apDevices10); // 10

  address.SetBase ("10.1.11.0", "255.255.255.0");
  address.Assign (staDevices11);
  address.Assign (apDevices11); // 11

  // Setting up the echo port
  UdpEchoServerHelper echoServer (9);
  // SERVER => The rightmost node
  ApplicationContainer serverApps = echoServer.Install (csmaNodes6.Get (nCsma));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
  // SERVER <=> CLIENT
  UdpEchoClientHelper echoClient (csmaInterfaces6.GetAddress (nCsma), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
  // CLIENTS => 10 for each network
  ApplicationContainer clientApps =
    echoClient.Install (wifiStaNodes8);
  clientApps.Add(echoClient.Install (wifiStaNodes9));
  clientApps.Add(echoClient.Install (wifiStaNodes10));
  clientApps.Add(echoClient.Install (wifiStaNodes11));
  clientApps.Add(echoClient.Install (csmaNodes6));
  clientApps.Add(echoClient.Install (csmaNodes7));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
  // Enabling internetwork
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Simulation
  Simulator::Stop (Seconds (10.0));

  if (tracing == true)
    {
      pointToPoint.EnablePcapAll ("proj_2_tr1");
      phy8.EnablePcap ("proj_2_tr1_8", apDevices8.Get (0));
      phy9.EnablePcap ("proj_2_tr1_9", apDevices9.Get (0));
      phy10.EnablePcap ("proj_2_tr1_10", apDevices10.Get (0));
      phy11.EnablePcap ("proj_2_tr1_11", apDevices11.Get (0));
      csma6.EnablePcap ("proj_2_tr1_6", csmaDevices6.Get (0), true);
      csma7.EnablePcap ("proj_2_tr1_7", csmaDevices7.Get (0), true);
    }

  // Install FlowMonitor on all nodes
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  NS_LOG_UNCOND(":::::::::: SIMULATION ::::::::::\n");
  // Run simulation for 10 seconds
  Simulator::Stop (Seconds (10));
  Simulator::Run ();

  // Print per flow statistics in xml file
  // flowmon->SerializeToXmlFile("statistics.xml", true, true);

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
