/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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

#include "ns3/wifi-net-device.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-phy.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/error-rate-model.h"
#include "ns3/yans-error-rate-model.h"
#include "ns3/nist-error-rate-model.h"
#include "ns3/dsss-error-rate-model.h"
#include "ns3/ptr.h"
#include "ns3/mobility-model.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/vector.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/command-line.h"
#include "ns3/flow-id-tag.h"
#include "ns3/wifi-tx-vector.h"

#include "sstream"

using namespace ns3;

class PsrExperiment
{
public:
  struct Input
  {
    Input ();
    double distance;
    std::string txMode;
    uint8_t txPowerLevel;
    uint32_t packetSize;
    uint32_t nPackets;
    //customize for different loss model
    std::string lossMd;
    //customize for different error rate
    std::string errRateMd;
    std::string wifiStand;
    std::string wifiClass;
  };
  struct Output
  {
    uint32_t received;
  };
  PsrExperiment ();

  struct PsrExperiment::Output Run (struct PsrExperiment::Input input);
  struct PsrExperiment::Output myRun (struct PsrExperiment::Input input);

private:
  void Send (void);
  void Receive (Ptr<Packet> p, double snr, WifiTxVector txVector, enum WifiPreamble preamble);
  Ptr<WifiPhy> m_tx;
  struct Input m_input;
  struct Output m_output;
};

void
PsrExperiment::Send (void)
{
  Ptr<Packet> p = Create<Packet> (m_input.packetSize);
  WifiMode mode = WifiMode (m_input.txMode);
  WifiTxVector txVector;
  txVector.SetTxPowerLevel (m_input.txPowerLevel);
  txVector.SetMode (mode);
  m_tx->SendPacket (p, txVector, WIFI_PREAMBLE_LONG);
}

void
PsrExperiment::Receive (Ptr<Packet> p, double snr, WifiTxVector txVector, enum WifiPreamble preamble)
{
  m_output.received++;
}

PsrExperiment::PsrExperiment ()
{
}
// modified
PsrExperiment::Input::Input ()
  : distance (5.0),
    txMode ("OfdmRate6Mbps"),
    txPowerLevel (0),
    packetSize (2304),
    nPackets (400),
    lossMd ("Log"),
    errRateMd ("Yans"),
    wifiStand ("80211a"),
    wifiClass ("ofdm")
{
}

struct PsrExperiment::Output
PsrExperiment::Run (struct PsrExperiment::Input input)
{
  m_output.received = 0;
  m_input = input;

  Ptr<MobilityModel> posTx = CreateObject<ConstantPositionMobilityModel> ();
  posTx->SetPosition (Vector (0.0, 0.0, 0.0));
  Ptr<MobilityModel> posRx = CreateObject<ConstantPositionMobilityModel> ();
  posRx->SetPosition (Vector (m_input.distance, 0.0, 0.0));

  Ptr<YansWifiChannel> channel = CreateObject<YansWifiChannel> ();
  channel->SetPropagationDelayModel (CreateObject<ConstantSpeedPropagationDelayModel> ());
  Ptr<LogDistancePropagationLossModel> log = CreateObject<LogDistancePropagationLossModel> ();
  channel->SetPropagationLossModel (log);

  Ptr<YansWifiPhy> tx = CreateObject<YansWifiPhy> ();
  Ptr<YansWifiPhy> rx = CreateObject<YansWifiPhy> ();
  Ptr<ErrorRateModel> error = CreateObject<YansErrorRateModel> ();
  tx->SetErrorRateModel (error);
  rx->SetErrorRateModel (error);
  tx->SetChannel (channel);
  rx->SetChannel (channel);
  tx->SetMobility (posTx);
  rx->SetMobility (posRx);

  tx->ConfigureStandard (WIFI_PHY_STANDARD_80211a);
  rx->ConfigureStandard (WIFI_PHY_STANDARD_80211a);

  rx->SetReceiveOkCallback (MakeCallback (&PsrExperiment::Receive, this));

  for (uint32_t i = 0; i < m_input.nPackets; ++i)
    {
      Simulator::Schedule (Seconds (i), &PsrExperiment::Send, this);
    }
  m_tx = tx;
  Simulator::Run ();
  Simulator::Destroy ();
  return m_output;
}
//***********************************************
//            modified
//*******@NTU

struct PsrExperiment::Output
PsrExperiment::myRun (struct PsrExperiment::Input input)
{
  m_output.received = 0;
  m_input = input;

  Ptr<MobilityModel> posTx = CreateObject<ConstantPositionMobilityModel> ();
  posTx->SetPosition (Vector (0.0, 0.0, 0.0));
  Ptr<MobilityModel> posRx = CreateObject<ConstantPositionMobilityModel> ();
  posRx->SetPosition (Vector (m_input.distance, 0.0, 0.0));

  Ptr<YansWifiChannel> channel = CreateObject<YansWifiChannel> ();
  //**** should change the propagation delay model
  channel->SetPropagationDelayModel (CreateObject<ConstantSpeedPropagationDelayModel> ());
  //**** should change the propagation loss model
  /**
  common functions:
  SetNext(Ptr<PropagationLossModel> next);
  Ptr<PropagationLossModel> GetNext();
  double CalcRxPower(double txPowerDbm,Ptr<MobilityModel> a,Ptr<MobilityModel> b)const;
  int64_t AssignStreams(int64_t stream);
  
  models to be used:
  */
  Ptr<PropagationLossModel> lossMd;
  /**
  1.RandomPropagationLossModel;
  
  2.FriisPropagationLossModel;
    void SetFrequency(double freq);
    void SetSystemLoss(double systemloss);
    void SetMinLoss(double minLoss);
    double GetMinLoss(void) const;
    double GetFrequency(void) const;
    double GetSystemLoss(void) const;
  
  3.TwoRayGroundPropagationLossModel;
    void SetFrequency(double freq);
    void SetSystemLoss(double sysloss);
    void SetMinDistance(double minDis);
    double GetMinDistance(void) const;
    double GetFrequency(void) const;
    double GetSystemLoss(void) const;
    void SetHeightAboveZ(double height);
  
  4.LogDistancePropagationLossModel;
    void SetPathLossExponent(double n);
    double GetPathLossExponent(void) const;
    void SetReference(double refDis, double refLoss);  
  
  5.ThreeLogDistancePropagationLossModel;
  
  6.NakagamiPropagationLossModel;
  
  7.FixedRssLossModel;
    SetRss(double rss);   //received signal strength (dBm)
  
  8.MatrixPropagationLossModel;
    void SetLoss(Ptr<MobilityModel> a, Ptr<MobilityModel> b, double loss, bool symmetric = true);
    void SetDefaultLoss(double loss);
  
  9.RangePropagationLossModel;
  */     
  
  // modified
  std::string lMd=m_input.lossMd;
  if(lMd=="Random"){
    lossMd = CreateObject<RandomPropagationLossModel> ();
  }
  else if(lMd=="Friis"){
    Ptr<FriisPropagationLossModel> f_ptr = CreateObject<FriisPropagationLossModel> ();
    f_ptr->SetFrequency(5.150e9);
    f_ptr->SetSystemLoss(1);
    f_ptr->SetMinLoss(0);
    lossMd=f_ptr;
  }
  else if(lMd=="TwoRay"){
    Ptr<TwoRayGroundPropagationLossModel> t_ptr = CreateObject<TwoRayGroundPropagationLossModel> ();
    t_ptr->SetFrequency(5.150e9);
    t_ptr->SetSystemLoss(1);
    t_ptr->SetMinDistance(0.5);
    t_ptr->SetHeightAboveZ(0);
    lossMd=t_ptr;
  }
  else if(lMd=="Log"){
    
    Ptr<LogDistancePropagationLossModel> l_ptr = CreateObject<LogDistancePropagationLossModel> ();
    l_ptr->SetPathLossExponent(3.0);
    l_ptr->SetReference(1.0,46.6777);
    lossMd=l_ptr;
  }
  else if(lMd=="ThreeLog"){
    lossMd = CreateObject<ThreeLogDistancePropagationLossModel> ();
  }
  else if(lMd=="Nakagami"){
    lossMd = CreateObject<NakagamiPropagationLossModel> ();
  }
  else if(lMd=="FixedRss"){
    Ptr<FixedRssLossModel> rss_ptr= CreateObject<FixedRssLossModel> ();
    rss_ptr->SetRss(150.0);
    lossMd=rss_ptr;
  }
  else if(lMd=="Matrix"){
    Ptr<MatrixPropagationLossModel> mat_ptr= CreateObject<MatrixPropagationLossModel> ();
    mat_ptr->SetLoss(posTx,posRx,100,true); 
    lossMd=mat_ptr;
  }
  else if(lMd=="Range"){
    lossMd = CreateObject<RangePropagationLossModel> ();
  }
  else{
    lossMd = CreateObject<LogDistancePropagationLossModel> ();
    /*
  std::cout << "apply default loss model: Log" << std::endl;
  std::cout << "Options: 1.Random 2.Friis 3.TwoRay 4.Log 5.ThreeLog" << std::endl;
  std::cout << "6.Nakagami 7.FixedRss 8.Matrix 9.Range" <<std::endl;
  */
  }
  channel->SetPropagationLossModel (lossMd);

  Ptr<YansWifiPhy> tx = CreateObject<YansWifiPhy> ();
  Ptr<YansWifiPhy> rx = CreateObject<YansWifiPhy> ();

  //**** should change the error rate model
  
  //Models to be used:
  /*
  1. NistErrorRateModel;
  2. DsssErrorRateModel; //option to be solve?
  3. YansErrorRateModel;
  */
  std::string errMd=m_input.errRateMd;
  Ptr<ErrorRateModel> error;
  if(errMd=="Yans"){
      error = CreateObject<YansErrorRateModel> ();
  }
  else if(errMd=="Nist"){
      error = CreateObject<NistErrorRateModel> ();
  }
  else{
      error = CreateObject<YansErrorRateModel> ();
      /*
      std::cout << "apply default error rate model: Yans" << std::endl;
      std::cout << "Options: 1.Yans 2.Nist" << std::endl;
      */
  }
  tx->SetErrorRateModel (error);
  rx->SetErrorRateModel (error);

  tx->SetChannel (channel);
  rx->SetChannel (channel);
  tx->SetMobility (posTx);
  rx->SetMobility (posRx);

  /** enum {0-8}
   OFDM PHY for the 5 GHz band (Clause 17) 
  1.WIFI_PHY_STANDARD_80211a,
  * DSSS PHY (Clause 15) and HR/DSSS PHY (Clause 18) 
  2.WIFI_PHY_STANDARD_80211b,
  * ERP-OFDM PHY (Clause 19, Section 19.5) 
  3.WIFI_PHY_STANDARD_80211g,
  * OFDM PHY for the 5 GHz band (Clause 17 with 10 MHz channel bandwidth) 
  4.WIFI_PHY_STANDARD_80211_10MHZ,
  * OFDM PHY for the 5 GHz band (Clause 17 with 5 MHz channel bandwidth) 
  5.WIFI_PHY_STANDARD_80211_5MHZ,
   * This is intended to be the configuration used in this paper:
   *  Gavin Holland, Nitin Vaidya and Paramvir Bahl, "A Rate-Adaptive
   *  MAC Protocol for Multi-Hop Wireless Networks", in Proc. of
   *  ACM MOBICOM, 2001.
  6.WIFI_PHY_STANDARD_holland,
  * HT OFDM PHY for the 2.4 GHz band (clause 20) 
  7.WIFI_PHY_STANDARD_80211n_2_4GHZ,
  * HT OFDM PHY for the 5 GHz band (clause 20) 
  8.WIFI_PHY_STANDARD_80211n_5GHZ,
  * VHT OFDM PHY (clause 22) 
  9.WIFI_PHY_STANDARD_80211ac
  */
  std::string u_wifi=m_input.wifiStand;
  WifiPhyStandard u1;
  if(u_wifi=="80211a"){
      u1=WIFI_PHY_STANDARD_80211a;
  }
  else if(u_wifi=="80211b"){
      u1=WIFI_PHY_STANDARD_80211b;
  }
  else if(u_wifi=="80211g"){
      u1=WIFI_PHY_STANDARD_80211g;
  }
  else if(u_wifi=="80211_10MHZ"){
      u1=WIFI_PHY_STANDARD_80211_10MHZ;
  }
  else if(u_wifi=="80211_5MHZ"){
      u1=WIFI_PHY_STANDARD_80211_5MHZ;
  }
  else if(u_wifi=="holland"){
      u1=WIFI_PHY_STANDARD_holland;
  }
  else if(u_wifi=="80211n_2_4GHZ"){
      u1=WIFI_PHY_STANDARD_80211n_2_4GHZ;
  }
  else if(u_wifi=="80211n_5GHZ"){
      u1=WIFI_PHY_STANDARD_80211n_5GHZ;
  }
  else if(u_wifi=="80211ac"){
      u1=WIFI_PHY_STANDARD_80211ac;
  }
  else{
      u1=WIFI_PHY_STANDARD_80211a;
  }

  //**** should change the WIFI mode
  tx->ConfigureStandard (u1);

  //**** should change the WIFI mode
  rx->ConfigureStandard (u1);

  rx->SetReceiveOkCallback (MakeCallback (&PsrExperiment::Receive, this));

  for (uint32_t i = 0; i < m_input.nPackets; ++i)
    {
      Simulator::Schedule (Seconds (i), &PsrExperiment::Send, this);
    }
  m_tx = tx;
  Simulator::Run ();
  Simulator::Destroy ();
  return m_output;
}

class CollisionExperiment
{
public:
  struct Input
  {
    Input ();
    Time interval;
    double xA;
    double xB;
    std::string txModeA;
    std::string txModeB;
    uint8_t txPowerLevelA;
    uint8_t txPowerLevelB;
    uint32_t packetSizeA;
    uint32_t packetSizeB;
    uint32_t nPackets;
    //customized
    std::string lossMd;
    std::string errRateMd;
    std::string wifiStand;
  };
  struct Output
  {
    uint32_t receivedA;
    uint32_t receivedB;
  };
  CollisionExperiment ();

  struct CollisionExperiment::Output Run (struct CollisionExperiment::Input input);
  struct CollisionExperiment::Output myRun (struct CollisionExperiment::Input input);
private:
  void SendA (void) const;
  void SendB (void) const;
  void Receive (Ptr<Packet> p, double snr, WifiTxVector txVector, enum WifiPreamble preamble);
  Ptr<WifiPhy> m_txA;
  Ptr<WifiPhy> m_txB;
  uint32_t m_flowIdA;
  uint32_t m_flowIdB;
  struct Input m_input;
  struct Output m_output;
};

void
CollisionExperiment::SendA (void) const
{
  Ptr<Packet> p = Create<Packet> (m_input.packetSizeA);
  p->AddByteTag (FlowIdTag (m_flowIdA));
  WifiTxVector txVector;
  txVector.SetTxPowerLevel (m_input.txPowerLevelA);
  txVector.SetMode (WifiMode (m_input.txModeA));
  m_txA->SendPacket (p, txVector, WIFI_PREAMBLE_LONG);
}

void
CollisionExperiment::SendB (void) const
{
  Ptr<Packet> p = Create<Packet> (m_input.packetSizeB);
  p->AddByteTag (FlowIdTag (m_flowIdB));
  WifiTxVector txVector;
  txVector.SetTxPowerLevel (m_input.txPowerLevelB);
  txVector.SetMode (WifiMode (m_input.txModeB));
  m_txB->SendPacket (p, txVector, WIFI_PREAMBLE_LONG);
}

void
CollisionExperiment::Receive (Ptr<Packet> p, double snr, WifiTxVector txVector, enum WifiPreamble preamble)
{
  FlowIdTag tag;
  if (p->FindFirstMatchingByteTag (tag))
    {
      if (tag.GetFlowId () == m_flowIdA)
        {
          m_output.receivedA++;
        }
      else if (tag.GetFlowId () == m_flowIdB)
        {
          m_output.receivedB++;
        }
    }
}

CollisionExperiment::CollisionExperiment ()
{
}
CollisionExperiment::Input::Input ()
  : interval (MicroSeconds (0)),
    xA (-5),
    xB (5),
    txModeA ("OfdmRate6Mbps"),
    txModeB ("OfdmRate6Mbps"),
    txPowerLevelA (0),
    txPowerLevelB (0),
    packetSizeA (2304),
    packetSizeB (2304),
    nPackets (400),
    lossMd ("Log"),
    errRateMd ("Yans"),
    wifiStand ("80211a")
{
}

struct CollisionExperiment::Output
CollisionExperiment::Run (struct CollisionExperiment::Input input)
{
  m_output.receivedA = 0;
  m_output.receivedB = 0;
  m_input = input;

  m_flowIdA = FlowIdTag::AllocateFlowId ();
  m_flowIdB = FlowIdTag::AllocateFlowId ();

  Ptr<YansWifiChannel> channel = CreateObject<YansWifiChannel> ();
  channel->SetPropagationDelayModel (CreateObject<ConstantSpeedPropagationDelayModel> ());
  Ptr<LogDistancePropagationLossModel> log = CreateObject<LogDistancePropagationLossModel> ();
  channel->SetPropagationLossModel (log);

  Ptr<MobilityModel> posTxA = CreateObject<ConstantPositionMobilityModel> ();
  posTxA->SetPosition (Vector (input.xA, 0.0, 0.0));
  Ptr<MobilityModel> posTxB = CreateObject<ConstantPositionMobilityModel> ();
  posTxB->SetPosition (Vector (input.xB, 0.0, 0.0));
  Ptr<MobilityModel> posRx = CreateObject<ConstantPositionMobilityModel> ();
  posRx->SetPosition (Vector (0, 0.0, 0.0));

  Ptr<YansWifiPhy> txA = CreateObject<YansWifiPhy> ();
  Ptr<YansWifiPhy> txB = CreateObject<YansWifiPhy> ();
  Ptr<YansWifiPhy> rx = CreateObject<YansWifiPhy> ();

  Ptr<ErrorRateModel> error = CreateObject<YansErrorRateModel> ();
  txA->SetErrorRateModel (error);
  txB->SetErrorRateModel (error);
  rx->SetErrorRateModel (error);
  txA->SetChannel (channel);
  txB->SetChannel (channel);
  rx->SetChannel (channel);
  txA->SetMobility (posTxA);
  txB->SetMobility (posTxB);
  rx->SetMobility (posRx);

  txA->ConfigureStandard (WIFI_PHY_STANDARD_80211a);
  txB->ConfigureStandard (WIFI_PHY_STANDARD_80211a);
  rx->ConfigureStandard (WIFI_PHY_STANDARD_80211a);

  rx->SetReceiveOkCallback (MakeCallback (&CollisionExperiment::Receive, this));

  for (uint32_t i = 0; i < m_input.nPackets; ++i)
    {
      Simulator::Schedule (Seconds (i), &CollisionExperiment::SendA, this);
    }
  for (uint32_t i = 0; i < m_input.nPackets; ++i)
    {
      Simulator::Schedule (Seconds (i) + m_input.interval, &CollisionExperiment::SendB, this);
    }
  m_txA = txA;
  m_txB = txB;
  Simulator::Run ();
  Simulator::Destroy ();
  return m_output;
}

struct CollisionExperiment::Output
CollisionExperiment::myRun (struct CollisionExperiment::Input input)
{
  m_output.receivedA = 0;
  m_output.receivedB = 0;
  m_input = input;

  m_flowIdA = FlowIdTag::AllocateFlowId ();
  m_flowIdB = FlowIdTag::AllocateFlowId ();

  Ptr<YansWifiChannel> channel = CreateObject<YansWifiChannel> ();
  channel->SetPropagationDelayModel (CreateObject<ConstantSpeedPropagationDelayModel> ());

  //**** should change propagation loss model
  std::string lMd = m_input.lossMd;
  Ptr<PropagationLossModel> lossMd;
  if(lMd=="Random"){
    lossMd = CreateObject<RandomPropagationLossModel> ();
  }
  else if(lMd=="Friis"){
    lossMd = CreateObject<FriisPropagationLossModel> ();
  }
  else if(lMd=="TwoRay"){
    lossMd = CreateObject<TwoRayGroundPropagationLossModel> ();
  }
  else if(lMd=="Log"){
    lossMd = CreateObject<LogDistancePropagationLossModel> ();
  }
  else if(lMd=="ThreeLog"){
    lossMd = CreateObject<ThreeLogDistancePropagationLossModel> ();
  }
  else if(lMd=="Nakagami"){
    lossMd = CreateObject<NakagamiPropagationLossModel> ();
  }
  else if(lMd=="FixedRss"){
    lossMd = CreateObject<FixedRssLossModel> ();
  }
  else if(lMd=="Matrix"){
    lossMd = CreateObject<MatrixPropagationLossModel> ();
  }
  else if(lMd=="Range"){
    lossMd = CreateObject<RangePropagationLossModel> ();
  }
  else{
    lossMd = CreateObject<LogDistancePropagationLossModel> ();
  }
  channel->SetPropagationLossModel (lossMd);

  Ptr<MobilityModel> posTxA = CreateObject<ConstantPositionMobilityModel> ();
  posTxA->SetPosition (Vector (input.xA, 0.0, 0.0));
  Ptr<MobilityModel> posTxB = CreateObject<ConstantPositionMobilityModel> ();
  posTxB->SetPosition (Vector (input.xB, 0.0, 0.0));
  Ptr<MobilityModel> posRx = CreateObject<ConstantPositionMobilityModel> ();
  posRx->SetPosition (Vector (0, 0.0, 0.0));

  Ptr<YansWifiPhy> txA = CreateObject<YansWifiPhy> ();
  Ptr<YansWifiPhy> txB = CreateObject<YansWifiPhy> ();
  Ptr<YansWifiPhy> rx = CreateObject<YansWifiPhy> ();

  //**** should change error rate model
  std::string errMd=m_input.errRateMd;
  Ptr<ErrorRateModel> error;
  if(errMd=="Yans"){
      error = CreateObject<YansErrorRateModel> ();
  }
  else if(errMd=="Nist"){
      error = CreateObject<NistErrorRateModel> ();
  }
  else{
      error = CreateObject<YansErrorRateModel> ();
      /*
      std::cout << "apply default error rate model: Yans" << std::endl;
      std::cout << "Options: 1.Yans 2.Nist" << std::endl;
      */
  }
  txA->SetErrorRateModel (error);
  txB->SetErrorRateModel (error);
  rx->SetErrorRateModel (error);
  txA->SetChannel (channel);
  txB->SetChannel (channel);
  rx->SetChannel (channel);
  txA->SetMobility (posTxA);
  txB->SetMobility (posTxB);
  rx->SetMobility (posRx);

  std::string u_wifi=m_input.wifiStand;
  WifiPhyStandard u1;
  if(u_wifi=="80211a"){
      u1=WIFI_PHY_STANDARD_80211a;
  }
  else if(u_wifi=="80211b"){
      u1=WIFI_PHY_STANDARD_80211b;
  }
  else if(u_wifi=="80211g"){
      u1=WIFI_PHY_STANDARD_80211g;
  }
  else if(u_wifi=="80211_10MHZ"){
      u1=WIFI_PHY_STANDARD_80211_10MHZ;
  }
  else if(u_wifi=="80211_5MHZ"){
      u1=WIFI_PHY_STANDARD_80211_5MHZ;
  }
  else if(u_wifi=="holland"){
      u1=WIFI_PHY_STANDARD_holland;
  }
  else if(u_wifi=="80211n_2_4GHZ"){
      u1=WIFI_PHY_STANDARD_80211n_2_4GHZ;
  }
  else if(u_wifi=="80211n_5GHZ"){
      u1=WIFI_PHY_STANDARD_80211n_5GHZ;
  }
  else if(u_wifi=="80211ac"){
      u1=WIFI_PHY_STANDARD_80211ac;
  }
  else{
      u1=WIFI_PHY_STANDARD_80211a;
  }
  txA->ConfigureStandard (u1);
  txB->ConfigureStandard (u1);
  rx->ConfigureStandard (u1);

  rx->SetReceiveOkCallback (MakeCallback (&CollisionExperiment::Receive, this));

  for (uint32_t i = 0; i < m_input.nPackets; ++i)
    {
      Simulator::Schedule (Seconds (i), &CollisionExperiment::SendA, this);
    }
  for (uint32_t i = 0; i < m_input.nPackets; ++i)
    {
      Simulator::Schedule (Seconds (i) + m_input.interval, &CollisionExperiment::SendB, this);
    }
  m_txA = txA;
  m_txB = txB;
  Simulator::Run ();
  Simulator::Destroy ();
  return m_output;
}

static void PrintPsr (int argc, char *argv[])
{
  PsrExperiment experiment;
  struct PsrExperiment::Input input;

  CommandLine cmd;
  cmd.AddValue ("Distance", "The distance between two phys", input.distance);
  cmd.AddValue ("PacketSize", "The size of each packet sent", input.packetSize);
  cmd.AddValue ("TxMode", "The mode to use to send each packet", input.txMode);
  cmd.AddValue ("NPackets", "The number of packets to send", input.nPackets);
  cmd.AddValue ("TxPowerLevel", "The power level index to use to send each packet", input.txPowerLevel);
  cmd.Parse (argc, argv);

  struct PsrExperiment::Output output;
  output = experiment.Run (input);

  double psr = output.received;
  psr /= input.nPackets;

  std::cout << psr << std::endl;
}

double CalcPsr (struct PsrExperiment::Output output, struct PsrExperiment::Input input)
{
  double psr = output.received;
  psr /= input.nPackets;
  return psr;
}

static void PrintPsrVsDistance (int argc, char *argv[])
{
  struct PsrExperiment::Input input;
  CommandLine cmd;
  cmd.AddValue ("TxPowerLevel", "The power level index to use to send each packet", input.txPowerLevel);
  cmd.AddValue ("TxMode", "The mode to use to send each packet", input.txMode);
  cmd.AddValue ("NPackets", "The number of packets to send", input.nPackets);
  cmd.AddValue ("PacketSize", "The size of each packet sent", input.packetSize);
  cmd.Parse (argc, argv);

  for (input.distance = 1.0; input.distance < 165; input.distance += 2.0)
    {
      std::cout << input.distance;
      PsrExperiment experiment;
      struct PsrExperiment::Output output;

      input.txMode = "OfdmRate6Mbps";
      output = experiment.Run (input);
      std::cout << " " << CalcPsr (output, input);

      input.txMode = "OfdmRate9Mbps";
      output = experiment.Run (input);
      std::cout << " " << CalcPsr (output, input);

      input.txMode = "OfdmRate12Mbps";
      output = experiment.Run (input);
      std::cout << " " << CalcPsr (output, input);

      input.txMode = "OfdmRate18Mbps";
      output = experiment.Run (input);
      std::cout << " " << CalcPsr (output, input);

      input.txMode = "OfdmRate24Mbps";
      output = experiment.Run (input);
      std::cout << " " << CalcPsr (output, input);

      input.txMode = "OfdmRate36Mbps";
      output = experiment.Run (input);
      std::cout << " " << CalcPsr (output, input);

      input.txMode = "OfdmRate48Mbps";
      output = experiment.Run (input);
      std::cout << " " << CalcPsr (output, input);

      input.txMode = "OfdmRate54Mbps";
      output = experiment.Run (input);
      std::cout << " " << CalcPsr (output, input);

      std::cout << std::endl;
    }
}

static void PrintSizeVsRange (int argc, char *argv[])
{
  double targetPsr = 0.05;
  struct PsrExperiment::Input input;
  CommandLine cmd;
  cmd.AddValue ("TxPowerLevel", "The power level index to use to send each packet", input.txPowerLevel);
  cmd.AddValue ("TxMode", "The mode to use to send each packet", input.txMode);
  cmd.AddValue ("NPackets", "The number of packets to send", input.nPackets);
  cmd.AddValue ("TargetPsr", "The psr needed to assume that we are within range", targetPsr);
  cmd.Parse (argc, argv);

  for (input.packetSize = 10; input.packetSize < 3000; input.packetSize += 40)
    {
      double precision = 0.1;
      double low = 1.0;
      double high = 200.0;
      while (high - low > precision)
        {
          double middle = low + (high - low) / 2;
          struct PsrExperiment::Output output;
          PsrExperiment experiment;
          input.distance = middle;
          output = experiment.Run (input);
          double psr = CalcPsr (output, input);
          if (psr >= targetPsr)
            {
              low = middle;
            }
          else
            {
              high = middle;
            }
        }
      std::cout << input.packetSize << " " << input.distance << std::endl;
    }
}

static void PrintPsrVsCollisionInterval (int argc, char *argv[])
{
  CollisionExperiment::Input input;
  input.nPackets = 100;
  CommandLine cmd;
  cmd.AddValue ("NPackets", "The number of packets to send for each transmitter", input.nPackets);
  cmd.AddValue ("xA", "the position of transmitter A", input.xA);
  cmd.AddValue ("xB", "the position of transmitter B", input.xB);
  cmd.Parse (argc, argv);

  for (uint32_t i = 0; i < 100; i += 1)
    {
      CollisionExperiment experiment;
      CollisionExperiment::Output output;
      input.interval = MicroSeconds (i);
      output = experiment.Run (input);
      double perA = (output.receivedA + 0.0) / (input.nPackets + 0.0);
      double perB = (output.receivedB + 0.0) / (input.nPackets + 0.0);
      std::cout << i << " " << perA << " " << perB << std::endl;
    }
  for (uint32_t i = 100; i < 4000; i += 50)
    {
      CollisionExperiment experiment;
      CollisionExperiment::Output output;
      input.interval = MicroSeconds (i);
      output = experiment.Run (input);
      double perA = (output.receivedA + 0.0) / (input.nPackets + 0.0);
      double perB = (output.receivedB + 0.0) / (input.nPackets + 0.0);
      std::cout << i << " " << perA << " " << perB << std::endl;
    }
}

void myUsage(){
    std::cout<< "USAGE: [options]:" << std::endl;
    std::cout<< "LossMd, "
             << "ErrRateMd, "
             << "TxPowerLevel, "
             << "NPackets, "
             << "PacketSize, "
             << "WifiStand, "
             << "WifiClass"
             << std::endl
             << "NOTE:"<< std::endl;
    std::cout<< "WifiStand: specify the standard followed by all users."<< std::endl;
    std::cout<< "WifiClass: specify the type of modes[Dsss, ErpOfdm, Ofdm, Ofdm_Bw10, Ofdm_Bw5, HtMcs, VhtMcs]" << std::endl;
}

static void PrintMyTest (int argc, char * argv[])
{
  PsrExperiment::Input input;
  CommandLine cmd;
  cmd.AddValue ("LossMd", "The propagation loss model in the experiment",input.lossMd);
  cmd.AddValue ("ErrRateMd","The error rate model in the experiment",input.errRateMd);
  cmd.AddValue ("TxPowerLevel", "The power level index to use to send each packet", input.txPowerLevel);
  cmd.AddValue ("TxMode", "The mode to use to send each packet", input.txMode);
  cmd.AddValue ("NPackets", "The number of packets to send", input.nPackets);
  cmd.AddValue ("PacketSize", "The size of each packet sent", input.packetSize);
  cmd.AddValue ("WifiStand","The wifi standard for users",input.wifiStand);
  cmd.AddValue ("WifiClass","The wifi modes in the experiment",input.wifiClass);
  cmd.Parse (argc, argv);

  std::vector<std::string> ofdmModes, ofdm10BwModes, ofdm5BwModes, dsssModes, erpOfdmModes, htMcsModes, vhtMcsModes;
  std::vector<std::string>* wifiClassPtr;
  //dsss
  dsssModes.push_back("DsssRate1Mbps");
  dsssModes.push_back("DsssRate2Mbps");
  dsssModes.push_back("DsssRate5_5Mbps");
  dsssModes.push_back("DsssRate11Mbps");
  //erpOfdm
  erpOfdmModes.push_back("ErpOfdmRate6Mbps");
  erpOfdmModes.push_back("ErpOfdmRate9Mbps");
  erpOfdmModes.push_back("ErpOfdmRate18Mbps");
  erpOfdmModes.push_back("ErpOfdmRate24Mbps");
  erpOfdmModes.push_back("ErpOfdmRate36Mbps");
  erpOfdmModes.push_back("ErpOfdmRate48Mbps");
  erpOfdmModes.push_back("ErpOfdmRate54Mbps");
  //ofdm
  ofdmModes.push_back("OfdmRate6Mbps");
  ofdmModes.push_back("OfdmRate9Mbps");
  ofdmModes.push_back("OfdmRate12Mbps");
  ofdmModes.push_back("OfdmRate18Mbps");
  ofdmModes.push_back("OfdmRate24Mbps");
  ofdmModes.push_back("OfdmRate36Mbps");
  ofdmModes.push_back("OfdmRate48Mbps");
  ofdmModes.push_back("OfdmRate54Mbps");
  //ofdm 10 bw
  ofdm10BwModes.push_back("OfdmRate3MbpsBW10MHz");
  ofdm10BwModes.push_back("OfdmRate4_5MbpsBW10MHz");
  ofdm10BwModes.push_back("OfdmRate6MbpsBW10MHz");
  ofdm10BwModes.push_back("OfdmRate9MbpsBW10MHz");
  ofdm10BwModes.push_back("OfdmRate12MbpsBW10MHz");
  ofdm10BwModes.push_back("OfdmRate18MbpsBW10MHz");
  ofdm10BwModes.push_back("OfdmRate24MbpsBW10MHz");
  ofdm10BwModes.push_back("OfdmRate27MbpsBW10MHz");
  //ofdm 5 bw
  ofdm5BwModes.push_back("OfdmRate1_5MbpsBW5MHz");
  ofdm5BwModes.push_back("OfdmRate2_25MbpsBW5MHz");
  ofdm5BwModes.push_back("OfdmRate3MbpsBW5MHz");
  ofdm5BwModes.push_back("OfdmRate4_5MbpsBW5MHz");
  ofdm5BwModes.push_back("OfdmRate6MbpsBW5MHz");
  ofdm5BwModes.push_back("OfdmRate9MbpsBW5MHz");
  ofdm5BwModes.push_back("OfdmRate12MbpsBW5MHz");
  ofdm5BwModes.push_back("OfdmRate13_5MbpsBW5MHz");
  //HtMcs
  for(int i=0;i<32;++i){
      std::stringstream temp;
      temp <<i;
      htMcsModes.push_back("HtMcs"+temp.str());
  }
  //VhtMcs
  for(int i=0;i<10;++i){
      std::stringstream temp;
      temp <<i;
      vhtMcsModes.push_back("VhtMcs"+temp.str());
  }
  std::string m_wifiClass = input.wifiClass;
  if(m_wifiClass=="Dsss"){
    wifiClassPtr = &dsssModes;
  }
  else if(m_wifiClass=="ErpOfdm"){
    wifiClassPtr = &erpOfdmModes;
  }
  else if(m_wifiClass=="Ofdm"){
    wifiClassPtr = &ofdmModes;
  }
  else if(m_wifiClass=="Ofdm_Bw10"){
    wifiClassPtr = &ofdm10BwModes;
  }
  else if(m_wifiClass=="Ofdm_Bw5"){
    wifiClassPtr = &ofdm5BwModes;
  }
  else if(m_wifiClass=="HtMcs"){
    wifiClassPtr = &htMcsModes;
  }
  else if(m_wifiClass=="VhtMcs"){
    wifiClassPtr = &vhtMcsModes;
  }
  else{
    wifiClassPtr = &ofdmModes;
  }
  std::cout << "Distance/ Psr for mode " << m_wifiClass << std::endl;
  for (input.distance = 1.0; input.distance < 165; input.distance += 2.0)
    {
      std::cout << input.distance;
      PsrExperiment experiment;
      struct PsrExperiment::Output output;
      for(size_t t=0;t<wifiClassPtr->size();++t){
          input.txMode = wifiClassPtr->at(t);
          output = experiment.myRun (input);
          std::cout << " " << CalcPsr (output, input);
      }
      std::cout << std::endl;
    }
}

int main (int argc, char *argv[])
{
  if (argc <= 1)
    {
      std::cout << "Available experiments: "
                << "Psr "
                << "SizeVsRange "
                << "PsrVsDistance "
                << "PsrVsCollisionInterval "
                << "myTest"
                << std::endl;
      myUsage();
      return 0;
    }
  std::string type = argv[1];
  argc--;
  argv[1] = argv[0];
  argv++;
  if (type == "Psr")
    {
      PrintPsr (argc, argv);
    }
  else if (type == "SizeVsRange")
    {
      PrintSizeVsRange (argc, argv);
    }
  else if (type == "PsrVsDistance")
    {
      PrintPsrVsDistance (argc, argv);
    }
  else if (type == "PsrVsCollisionInterval")
    {
      PrintPsrVsCollisionInterval (argc, argv);
    }
  else if (type == "myTest")
    {
      PrintMyTest(argc, argv);
    }
  else
    {
      std::cout << "Wrong arguments!" << std::endl;
    }

  return 0;
}
