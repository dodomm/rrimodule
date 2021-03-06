/**
*          __________________Server_________
*         |			|		|               |
*        AP1         AP2     AP3          AP4
*                 _____|
*
*          Client0
*
*


There 1 client , 4 Aps and a server.
The 4 Aps and server are connected by p2p link.
1 client, 4 Aps and the server are stationary.


Aps Send beacons at different time intervals.

Aps are deployed on Channel 36 each AP is assigned a unique ssid
Ap1  --> ssid AP1
Ap2  --> ssid AP2
Ap3  --> ssid AP3
Ap4  --> ssid AP4


Two NetDevices are installed on the client node.
The RRI object/Measurement MAC is created and is installed on the second device
The second net device is not connected to any AP

The Measurement MAC object has  logic to make the measurements and scan the channels

1)From the script : Associate the client to AP with maximum snr (Initially client is not associated to any AP)

Create function to associate with new AP based on snr :
AssociateWithBestSNR() is scheduled to be called  at some time (5th second ) from the  script


Each AP is assigned a unique ssid value. This information is stored in a datastructure.
Change in association is based on the ssid of the AP.

Data traffic flows from the server to the client


To Run: ./waf --run scratch/twoRadioSnrAssn

Standard Used: IEEE 802.11ac

802.11ac configuration parameters:

short_guard enabled  enabled
vhtmcs0

AssociatewithBestSnr() is triggered for client at 5 seconds

*/

#include <iostream>
#include "ns3/netanim-module.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/gnuplot.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/olsr-module.h"

#include <iomanip>
#include <map>

using namespace ns3;
using namespace std;


NS_LOG_COMPONENT_DEFINE ("twoRadio");

static void
Connection (std::string context,Mac48Address bssid)
{
  NS_LOG_INFO ("Association at  " << Simulator::Now ().GetSeconds () << " to " << bssid);
}

static void
ShowDeassociation (std::string context,Mac48Address bssid)
{
  NS_LOG_INFO ("Deassociation at " << Simulator::Now ().GetSeconds () << " from " << bssid);
}

//Function to display the map containing average SNR values of last 5 beaons.
//We display both MAC of Aps and the average snr values.
//Also display the Aps and the channel number of the Aps
void displayStnSnr (Ptr<RriModule> rriMod)
{
  int chNo;
  Ssid apSsid;
  double snrValue;
  char* ssidVal;
  SnrMsrmtDetails msrDet;


  cout << endl;
  cout << "For Client scanned Radio Mac Address = "  << rriMod->GetAddress () << endl;
  cout << "-------------------------------------------------------------" << endl;

  std::map <Mac48Address, std::list<SnrMsrmtDetails> > mapApSnrSsidChnl = rriMod->GetSnrDetails ();

/* Create an iterator */
  std::map <Mac48Address, std::list<SnrMsrmtDetails > > ::iterator it1;

  std::list<SnrMsrmtDetails>::iterator it2;
  std::list<SnrMsrmtDetails> currentList;

  cout << endl;
  cout << "\tTotal size of Mapbssnr : " << mapApSnrSsidChnl.size () << endl; /* Output the size */

  cout << "AP Mac Address : \t  Av Snr \t Channel No \t  Ssid" << endl;
  cout << "------------------------------------------------------- --------" << endl;
  cout << endl;
  for (it1 = mapApSnrSsidChnl.begin (); it1 != mapApSnrSsidChnl.end (); ++it1)
    {
      currentList = mapApSnrSsidChnl[it1->first];
      double sumSnr = 0,snrAverage = 0;

      for ( it2 = currentList.begin (); it2 != currentList.end (); ++it2)
        {
          msrDet = *it2;
          chNo = msrDet.chnlNum;
          snrValue = msrDet.snrValue;
          sumSnr  = sumSnr  + snrValue;
          apSsid = msrDet.apSsid;
          ssidVal = apSsid.PeekString ();
        }

      snrAverage = sumSnr / currentList.size ();
      /* Output first , macid , snr value , channel no, ssid */
      cout << setw (20) << it1->first << "\t"  << setw (10) << snrAverage << " \t \t" << chNo << "\t" << ssidVal << "\n";

    }
  cout << endl; /* Print a new line */
}


//Function to display the map with client-AP association

void displayLoadDetails (Ptr<RriModule> rriMod)
{
  Mac48Address apMac;
  int        chnlNum;
  std::string  dataPath;
  LoadMsrmtDetails msrDet;

  cout << "Display Load Details " << "\n";
  cout << endl;
  cout << "For Client scanned Radio Mac Address = "  << rriMod->GetAddress () << endl;
  cout << "-------------------------------------------------------------" << endl;

  std::map <Mac48Address, LoadMsrmtDetails > mapclientApChannel = rriMod->GetLoadDetails ();

/* Create an iterator */
  std::map <Mac48Address, LoadMsrmtDetails > ::iterator it1;

  cout << endl;

  cout << "Client Mac Address : \t  Ap  Mac \t Channel No \t  UL/DL " << endl;
  cout << "------------------------------------------------------- --------" << endl;
  cout << endl;
  for (it1 = mapclientApChannel.begin (); it1 != mapclientApChannel.end (); ++it1)
    {

      msrDet =   it1->second;

      chnlNum = msrDet.chnlNum;
      apMac = msrDet.apMac;
      dataPath = msrDet.dataPath;

/* Output client macid , apMac , channel no*/
      cout << setw (20) << it1->first << "\t"  << setw (10) << apMac << " \t" << chnlNum << "\t" << dataPath << "\n";

    }

  cout << endl; /* Print a new line */
}

// Function to Associate with the AP with best snr value
void AssociateWithBestSNR (Ptr<StaWifiMac> staMac1, Ptr<RriModule> rriMod)
{


  int chNo;
  Ssid apSsid;
  double snrValue;
  char* ssidVal;
  SnrMsrmtDetails msrDet;


  std::cout << "\n";
  std::cout << "Inside Associate with Best: (" << staMac1->GetAddress () << ")" << std::endl;
  std::cout << "*************************** ";
  std::cout << "\n";


  std::map <Mac48Address, std::list<SnrMsrmtDetails> > mapApSnrSsidChnl = rriMod->GetSnrDetails ();

/* Create an iterator */
  std::map <Mac48Address, std::list<SnrMsrmtDetails > > ::iterator it1;

  std::list<SnrMsrmtDetails>::iterator it2;
  std::list<SnrMsrmtDetails> currentList;

  Mac48Address temp_add;
  double max = 0;

  for (it1 = mapApSnrSsidChnl.begin (); it1 != mapApSnrSsidChnl.end (); ++it1)
    {
      currentList = mapApSnrSsidChnl[it1->first];
      double sum_snr = 0,snr_average = 0;

      for ( it2 = currentList.begin (); it2 != currentList.end (); ++it2)
        {
          msrDet = *it2;
          chNo = msrDet.chnlNum;
          snrValue = msrDet.snrValue;
          sum_snr  = sum_snr  + snrValue;
          apSsid = msrDet.apSsid;
          ssidVal = apSsid.PeekString ();
        }

      snr_average = sum_snr / currentList.size ();

      std::cout << "MAC Address Of AP: " << it1->first << " ; " << "SINR Value: " << snr_average <<  " Channel Num: " << chNo << std::endl;
      // Check the SNR value
      if (( snr_average) > max)
        {
          temp_add = it1->first;
          max = snr_average;
        }
    }

  std::cout << "\n";

  if (temp_add != staMac1->GetBssid ())
    {

      msrDet = *( mapApSnrSsidChnl[temp_add].begin ());

      chNo = msrDet.chnlNum;
      apSsid = msrDet.apSsid;
      ssidVal = apSsid.PeekString ();

      staMac1->GetWifiPhy ()->SetChannelNumber (chNo);

      // Associate to the AP using ssid

      std::cout << "Client associating to AP with maximum SNR: " << max << "\n";
      std::cout << "****************************************************" << std::endl;

      std::cout << " MAC address \t Channel No, \t SSid: "  << "\n";

      std::cout << temp_add << "\t" << chNo << "\t" << ssidVal << "\n" << std::endl;

      Ptr<RegularWifiMac> regStmac =  DynamicCast<RegularWifiMac> ( staMac1 );
      regStmac->SetSsid (apSsid);

    }
  else
    {
      std::cout << "Client already associated to the AP with MAC address: " << temp_add << "\n";
      std::cout << "\n";
    }


}

int  main (int argc, char *argv[])
{

  LogComponentEnable ("twoRadio",LOG_LEVEL_INFO);



  double start_scanning = 2.0;
  double scan_duration = 1.0;
  std::string channelList = "36,36,36,36";


  CommandLine cmd;
  cmd.AddValue ("StartRRM", "Start Time of RRI Module", start_scanning);
  cmd.AddValue ("ScanDurtion", "Duration to stay in channel", scan_duration);
  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create 4 nodes.");

// server node
  NodeContainer ServerNode;
  ServerNode.Create (1);

// Create Wifi Aps and client
  NodeContainer wifiApNode; // 4 APs
  wifiApNode.Create (4);

//  Create p2p link: Server and AP1, server and AP2, server and AP3
  NodeContainer n0n1 = NodeContainer (ServerNode.Get (0), wifiApNode.Get (0)); // server and Ap1
  NodeContainer n0n2 = NodeContainer (ServerNode.Get (0), wifiApNode.Get (1)); // server and Ap2
  NodeContainer n0n3 = NodeContainer (ServerNode.Get (0), wifiApNode.Get (2)); // server and Ap3
  NodeContainer n0n4 = NodeContainer (ServerNode.Get (0), wifiApNode.Get (3)); // server and Ap4


  NodeContainer wifiStaNode;
  wifiStaNode.Create (1); // 1 clients

// create p2p links
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("1Gbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("1us"));

// install p2p device between server and ap1 , server and ap2 and server and ap3, server and ap4
  NetDeviceContainer p2pDevices1 = p2p.Install (n0n1);
  NetDeviceContainer p2pDevices2 = p2p.Install (n0n2);
  NetDeviceContainer p2pDevices3 = p2p.Install (n0n3);
  NetDeviceContainer p2pDevices4 = p2p.Install (n0n4);


// Enable the beacon Jitter for the AP Macs
  Config::SetDefault ("ns3::ApWifiMac::EnableBeaconJitter",BooleanValue (true));

// We create the channels first
  NS_LOG_INFO ("Create channels.");

  YansWifiChannelHelper channel;
  channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  channel.AddPropagationLoss ("ns3::FriisPropagationLossModel");


  YansWifiPhyHelper phy =  YansWifiPhyHelper::Default ();
  phy.Set ("RxGain", DoubleValue (0));
  phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
  phy.Set ("ShortGuardEnabled", BooleanValue (true));
  phy.Set ("ChannelNumber", UintegerValue (36) );
// create the channel object and associate to PHY layer object
  phy.SetChannel (channel.Create ());


// Set Wifi Standard
/*
WifiHelper wifi;
wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);

StringValue phyRate;
phyRate = StringValue ("VhtMcs0");
*/

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);

  StringValue phyRate;
  phyRate = StringValue ("OfdmRate6Mbps");


  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyRate),
                                "ControlMode",StringValue (phyRate));


//**********To set High Throughput wifi mac
/*
VhtWifiMacHelper mac = VhtWifiMacHelper::Default ();
VhtWifiMacHelper msrmac = VhtWifiMacHelper::Default ();
*/


  HtWifiMacHelper mac = HtWifiMacHelper::Default ();
  HtWifiMacHelper msrmac = HtWifiMacHelper::Default ();


  NetDeviceContainer wifistaDevice[2];
  Ssid ssid1 = Ssid ("AP1");
  Ssid ssid2 = Ssid ("AP2");
  Ssid ssid3 = Ssid ("AP3");
  Ssid ssid4 = Ssid ("AP4");
  Ssid ssid5 = Ssid ("invalid");


/******** Client 1 in ssid5 invalid ************/

  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid5),
               "ActiveProbing", BooleanValue (false),
               "QosSupported", BooleanValue (true));

/// Install Wifi device on the client Node.
  wifistaDevice[0] = wifi.Install (phy, mac, wifiStaNode.Get (0)); // Orig Mac

/// Install the Measuremnt Mac  on  client Node
  msrmac.SetType ("ns3::RriModule",
                  "StartTime", TimeValue (Seconds (start_scanning)),
                  "ScanDuration", TimeValue (Seconds (scan_duration)),
                  "ChannelToScan",StringValue ( channelList) );

  wifistaDevice[1] = wifi.Install (phy, msrmac, wifiStaNode.Get (0)); // Msr Mac

// Set the NetDevice of the msr mac to promiscous mode to receive data packets meant to device 1
  Ptr<WifiNetDevice> wifiNet =   DynamicCast<WifiNetDevice> (wifistaDevice[1].Get (0));
  wifiNet->GetMac ()->SetPromisc ();

  Ptr<RriModule> rriMod = DynamicCast<RriModule> (wifiNet->GetMac () );

//Call function on the scanned mac radio to pass list of channels to scan
// rriMod->SetChanneltoScan (chnl);
  rriMod->SetChanneltoScan ();

/****** Client1 ***********/


/* Setting Attributes for AP */

  mac.SetType ("ns3::ApWifiMac",
               "Ssid",SsidValue (ssid1),
               "BeaconGeneration", BooleanValue (true),
               "BeaconInterval",TimeValue (MicroSeconds (100000)),
               "QosSupported", BooleanValue (true));

  NetDeviceContainer wifiapDevice[4];

//APs 1,2,3,4 are in channel 36.
  wifiapDevice[0] = wifi.Install (phy,mac,wifiApNode.Get (0)); //Add the mobile devices for AP1
  Ptr<WifiNetDevice> apNetDev1 = DynamicCast<WifiNetDevice> (wifiapDevice[0].Get (0));
  Ptr<RegularWifiMac> apMac1 =  DynamicCast<RegularWifiMac> ( DynamicCast<ApWifiMac> (apNetDev1->GetMac () ) );
  apMac1->SetSsid (ssid1);


  wifiapDevice[1] = wifi.Install (phy,mac,wifiApNode.Get (1)); //Add the mobile devices for AP2
  Ptr<WifiNetDevice> apNetDev2 = DynamicCast<WifiNetDevice> (wifiapDevice[1].Get (0));
  Ptr<RegularWifiMac> apMac2 =  DynamicCast<RegularWifiMac> ( DynamicCast<ApWifiMac> (apNetDev2->GetMac () ) );
  apMac2->SetSsid (ssid2);


  wifiapDevice[2] = wifi.Install (phy,mac,wifiApNode.Get (2)); //Add the mobile devices for AP3
  Ptr<WifiNetDevice> apNetDev3 = DynamicCast<WifiNetDevice> (wifiapDevice[2].Get (0));
  Ptr<RegularWifiMac> apMac3 =  DynamicCast<RegularWifiMac> ( DynamicCast<ApWifiMac> (apNetDev3->GetMac () ) );
  apMac3->SetSsid (ssid3);

  wifiapDevice[3] = wifi.Install (phy,mac,wifiApNode.Get (3)); //Add the mobile devices for AP4
  Ptr<WifiNetDevice> apNetDev4 = DynamicCast<WifiNetDevice> (wifiapDevice[3].Get (0));
  Ptr<RegularWifiMac> apMac4 =  DynamicCast<RegularWifiMac> ( DynamicCast<ApWifiMac> (apNetDev4->GetMac () ) );
  apMac4->SetSsid (ssid4);


//Mobility models to set the position of the nodes
  MobilityHelper  mobility1;
  Ptr<ListPositionAllocator> positionAlloc1 = CreateObject<ListPositionAllocator> ();

  positionAlloc1->Add (Vector (0.0, 0.0, 0.0)); // server
  positionAlloc1->Add (Vector (0.0, 10.0, 0.0)); //Initial position of AP1
  positionAlloc1->Add (Vector (50.0, 10.0, 0.0)); //Initial position of the Ap2
  positionAlloc1->Add (Vector (100.0, 10.0, 0.0)); //Initial position of the Ap3
  positionAlloc1->Add (Vector (150.0, 10.0, 0.0)); //Initial position of the Ap4

  positionAlloc1->Add (Vector (40.0, 15.0, 0.0)); //Initial position of client1
  mobility1.SetPositionAllocator (positionAlloc1);

  mobility1.SetMobilityModel ("ns3::ConstantPositionMobilityModel"); //For stationary APs Aand client
  mobility1.Install (ServerNode);
  mobility1.Install (wifiApNode);
  mobility1.Install (wifiStaNode.Get (0)); // 1 Client and 3 Aps are stationary



//Using OLSR routing protocol
  OlsrHelper olsr;
  Ipv4StaticRoutingHelper staticRouting;
  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr, 10);

//Installing internet stack
  InternetStackHelper stack;
  stack.SetRoutingHelper (list);

  stack.Install (ServerNode);
  stack.Install (wifiStaNode);
  stack.Install (wifiApNode);

  NS_LOG_INFO ("Assign IP Addresses.");


  Ipv4AddressHelper address;

// ipaddress for p2p interfaces
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces1;
  p2pInterfaces1 = address.Assign (p2pDevices1);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces2 = address.Assign (p2pDevices2);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces3 = address.Assign (p2pDevices3);

  address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces4 = address.Assign (p2pDevices4);

// Assign IP address to the 2 interfaces on station nodes
  address.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterface[2];
  StaInterface[0] = address.Assign (wifistaDevice[0]); // Orig Mac
  StaInterface[1] = address.Assign (wifistaDevice[1]); // Msr Mac

  Ipv4InterfaceContainer ApInterface[4];
  ApInterface[0] = address.Assign (wifiapDevice[0]);
  ApInterface[1] = address.Assign (wifiapDevice[1]);
  ApInterface[2] = address.Assign (wifiapDevice[2]);
  ApInterface[3] = address.Assign (wifiapDevice[3]);


// Data connections from Server to client
  NS_LOG_INFO ("Create Applications.");
  uint16_t port = 9;


//Install the ON/OFF application downlink
  ApplicationContainer sourceApp1;

  // install source for the client 1 on Server
  AddressValue remoteAddress1;
  remoteAddress1   =  AddressValue (InetSocketAddress (StaInterface[0].GetAddress (0), port));


  BulkSendHelper bulkSend ("ns3::TcpSocketFactory", Address ());
  bulkSend.SetAttribute ("MaxBytes", UintegerValue (0));
  
   bulkSend.SetAttribute ("Remote", remoteAddress1);
   sourceApp1.Add (bulkSend.Install (ServerNode.Get (0)));
 
   sourceApp1.Start (Seconds (15.0));
  sourceApp1.Stop (Seconds (25.0));



// Create an optional packet sink to receive these packets on

  ApplicationContainer sink1;

  AddressValue localaddress;

  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", Address ());
  localaddress = Address (InetSocketAddress (Ipv4Address::GetAny (), port));
  packetSinkHelper.SetAttribute ("Local", localaddress);
  sink1.Add (packetSinkHelper.Install (wifiStaNode.Get (0))); // Install sinks on clients
  sink1.Start (Seconds (15.0));
  sink1.Stop (Seconds (25.0));


// Tracing, .pcap and .xml files generation
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("twoRadio.tr");
  stack.EnableAsciiIpv4All (stream);

  //phy.EnablePcapAll ("twoRadio");
  phy.EnablePcap ("twoRadio", wifistaDevice[0].Get (0));
  AnimationInterface anim ("twoRadio.xml");


/******* Client 1 *******************/
// Get pointer to the original station mac object for client 1
  Ptr<WifiNetDevice> stNetDev1 = DynamicCast<WifiNetDevice> (wifistaDevice[0].Get (0));
  Ptr<StaWifiMac> staMac1 =   DynamicCast<StaWifiMac> (stNetDev1->GetMac () );

// Get pointer to the new station mac object for client 1
  Ptr<WifiNetDevice> stNetDev2 = DynamicCast<WifiNetDevice> (wifistaDevice[1].Get (0));
  Ptr<RriModule> rriMod1 =   DynamicCast<RriModule> (stNetDev2->GetMac () );

// Display the measurement
  Simulator::Schedule (Seconds (4.9), &displayStnSnr, rriMod1);

//Associate with AP with best Snr
  Simulator::Schedule (Seconds (5), &AssociateWithBestSNR, staMac1, rriMod1 );

  Simulator::Schedule (Seconds (5.1), &displayStnSnr, rriMod1);

 /******* Client 1 *******************/

  Simulator::Stop (Seconds (30.0));

//This is to use the assoc trace source
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::StaWifiMac/Assoc",MakeCallback (&Connection));
  Config::Connect ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/$ns3::StaWifiMac/DeAssoc",MakeCallback (&ShowDeassociation));

  Simulator::Run ();

// Calculating throughput on  client
  double thrpt;


  uint32_t totalPacketsThrough = DynamicCast<PacketSink> (sink1.Get (0))->GetTotalRx ();
  std::cout << "Total Packets received on client 1 = " << totalPacketsThrough << endl;
  thrpt = totalPacketsThrough * 8 / (10 * 1000.0);
  std::cout << " Thrpt of client 1  "   << " Kbps =  " << thrpt << endl;


  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
