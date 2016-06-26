/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/athstats-helper.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/config-store-module.h"

#include <iostream>


using namespace ns3;

int main (int argc, char *argv[])
{
  RngSeedManager::SetSeed(3);
  RngSeedManager::SetRun(7);
  uint32_t packetSize = 1023;
  uint32_t nStas = 2;
  double radius = 5.0;
  double simDur = 9.0;
  CommandLine cmd;
  cmd.AddValue ("packetSize", "PacketSize in bytes", packetSize);
  cmd.AddValue ("nStas", "Number of Wifi stations", nStas);
  cmd.AddValue ("radius", "The Radius of Uniform Distributed STA Positions", nStas);
  cmd.AddValue ("Duration", "Simulation Duration in Seconds", simDur); 
  
  cmd.Parse (argc, argv);

  Packet::EnablePrinting ();

  // enable rts cts all the time.
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // disable fragmentation
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));

  WifiHelper wifi;
  MobilityHelper mobility;
  NodeContainer stas;
  NodeContainer ap;
  NetDeviceContainer staDevs;

  stas.Create (nStas);
  ap.Create (1);

  WifiMacHelper wifiMac;
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  Ssid ssid = Ssid ("wifi-default");

  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue ("DsssRate11Mbps"),
                                "ControlMode", StringValue ("DsssRate2Mbps"));
  //wifi.SetRemoteStationManager ("ns3::ArfWifiManager");
  // setup stas.
  wifiMac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid),
                   "ActiveProbing", BooleanValue (false));
  staDevs = wifi.Install (wifiPhy, wifiMac, stas);
  // setup ap.
  wifiMac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid));
  NetDeviceContainer apDev = wifi.Install (wifiPhy, wifiMac, ap);

  NetDeviceContainer infraDevices (apDev, staDevs);

  // mobility.
  /*
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  */
  Ptr<ListPositionAllocator> posPtr = CreateObject<ListPositionAllocator> ();
  posPtr->Add (Vector ( 0.0, 0.0, 0.0));
  mobility.SetPositionAllocator (posPtr);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ap);
  Ptr<UniformDiscPositionAllocator> staPos = CreateObject<UniformDiscPositionAllocator> ();
  staPos->SetRho (radius);
  staPos->SetX (0.0);
  staPos->SetY (0.0);
  mobility.SetPositionAllocator (staPos);
  mobility.Install (stas);

  InternetStackHelper internet;
  internet.Install (ap);
  internet.Install (stas);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("192.168.0.0", "255.255.255.0");
  Ipv4InterfaceContainer apInterface = ipv4.Assign (apDev);

  Ipv4InterfaceContainer staInterface = ipv4.Assign (staDevs);

//ONOFF APP
  uint32_t port=9;
  PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (apInterface.GetAddress(0), port));
  ApplicationContainer apps_sink = sink.Install (ap);
  
  OnOffHelper onoff ("ns3::UdpSocketFactory", InetSocketAddress (apInterface.GetAddress(0), port));
  onoff.SetConstantRate (DataRate ("54Mb/s"), packetSize);
  onoff.SetAttribute ("StartTime", TimeValue (Seconds (1.0)));
  onoff.SetAttribute ("StopTime", TimeValue (Seconds (1.0+simDur)));
  ApplicationContainer app_source = onoff.Install (stas);
  
  apps_sink.Start (Seconds (1.0));
  apps_sink.Stop (Seconds (1.0+simDur));


//UDP SERVER CLIENT APP
/*
  UdpServerHelper server(9);

  ApplicationContainer serverApps = server.Install (ap);
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpClientHelper client (apInterface.GetAddress (0), 9);
  client.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
  client.SetAttribute ("Interval", TimeValue (Time("0.0002")));
  client.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = client.Install (stas);
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
*/

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
  
  Simulator::Stop (Seconds (1.0+simDur));

  Simulator::Run ();


  // print position example
  std::cout << "Results\n";
  std::cout << "----------------------------------------------------------\n";
  Ptr<Node> bpt = ap.Get(0);
  Ptr<MobilityModel> mb = bpt->GetObject<MobilityModel> ();
  std::cout << "  AP   position: " << mb->GetPosition () << "\n";
  for(uint32_t i = 0; i < nStas; ++i)
    {
      bpt = stas.Get(i);
      mb = bpt->GetObject<MobilityModel> ();
      std::cout << "  sta" << i << " position: " << mb->GetPosition () << "\n";
    }
  std::cout << "----------------------------------------------------------\n";
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i != stats.end(); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      std::cout << "Flow " << i->first << " (" << t.sourceAddress << " ->" << t.destinationAddress << ")\n";
      std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
      std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
      std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / simDur / 1000 / 1000  << " Mbps\n";
      std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
      std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / simDur / 1000 / 1000  << " Mbps\n";
      std::cout << "  Delay:      " << i->second.delaySum.GetMilliSeconds() / i->second.rxPackets << " ms\n";
    }

  Simulator::Destroy ();
  return 0;
}
