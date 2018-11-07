/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/* *
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
 * Author: Davide Peron <davideperon94@gmail.com>
 */

#include "ns3/point-to-point-module.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mmwave-wave-ocb-wifi-mac.h"
#include "ns3/wifi-module.h"
#include "ns3/mmwave-wifi-80211p-helper.h"
#include "ns3/mmwave-wave-mac-helper.h"
#include "ns3/mmwave-wave-bsm-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include <ns3/buildings-helper.h>
#include <ns3/buildings-module.h>
#include "ns3/flow-monitor-module.h"
#include <ns3/packet.h>
#include <ns3/tag.h>
// #include "wifi-tx-vector.h"

using namespace ns3;

/**
 * A script to simulate the DOWNLINK TCP data over mmWave links
 * with the mmWave devices and the LTE EPC.
 */
NS_LOG_COMPONENT_DEFINE ("waveUDPExample");

static Ptr<OutputStreamWrapper> streamRxOk;
// static Ptr<OutputStreamWrapper> streamRxDrop;
static Ptr<OutputStreamWrapper> streamTx;
static Ptr<OutputStreamWrapper> streamSNR;
static Ptr<OutputStreamWrapper> streamPER;
/**
 * \ingroup wave
 * \brief The WifiPhyStats class collects Wifi MAC/PHY statistics
 */
class WifiPhyStats : public Object
{
public:
  /**
   * \brief Gets the class TypeId
   * \return the class TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor
   * \return none
   */
  WifiPhyStats ();

  /**
   * \brief Destructor
   * \return none
   */
  virtual ~WifiPhyStats ();

  /**
   * \brief Returns the number of bytes that have been transmitted
   * (this includes MAC/PHY overhead)
   * \return the number of bytes transmitted
   */
  uint32_t GetTxBytes ();

  uint32_t GetRxBytes ();

  /**
   * \brief Callback signiture for Phy/Tx trace
   * \param context this object
   * \param packet packet transmitted
   * \param mode wifi mode
   * \param preamble wifi preamble
   * \param txPower transmission power
   * \return none
   */
  void MacTxTrace (std::string context, Ptr<const Packet> packet, Ptr< Ipv4> ipv4, uint32_t interface);

  void MacRxOkTrace (std::string context, Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t interface);

  // void IpRxDropTrace (std::string context, const Ipv4Header &header, Ptr<const Packet> packet, Ipv4L3Protocol::DropReason reason, Ptr< Ipv4> ipv4, uint32_t interface);

  // void MacRxOkTrace (std::string context, Ptr<const Packet> packet);

  void PhySnifferRxTrace(std::string context, Ptr< const Packet > packet, uint16_t channelFreqMhz, WifiTxVector txVector, MpduInfo aMpdu, SignalNoiseDbm signalNoise, double snr);

private:
  uint32_t m_phyTxPkts; ///< phy transmit packets
  uint32_t m_phyTxBytes; ///< phy transmit bytes
  uint32_t m_phyRxPkts; ///< phy received packets
  uint32_t m_phyRxBytes; ///< phy received bytes
};

NS_OBJECT_ENSURE_REGISTERED (WifiPhyStats);

TypeId
WifiPhyStats::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WifiPhyStats")
  .SetParent<Object> ()
  .AddConstructor<WifiPhyStats> ();
  return tid;
}

WifiPhyStats::WifiPhyStats ()
: m_phyTxPkts (0),
m_phyTxBytes (0),
m_phyRxPkts (0),
m_phyRxBytes (0)
{
}

WifiPhyStats::~WifiPhyStats ()
{
}

void
WifiPhyStats::MacTxTrace (std::string context, Ptr<const Packet> packet, Ptr< Ipv4> ipv4, uint32_t interface)
{

  if(packet->GetSize() > 1000){
    *streamTx->GetStream () << Simulator::Now().GetSeconds() << "\t" << packet->GetUid() << "\t" << packet->GetSize() << std::endl;
  }

  ++m_phyTxPkts;
  uint32_t pktSize = packet->GetSize ();
  m_phyTxBytes += pktSize;

  //NS_LOG_UNCOND ("Received PHY size=" << pktSize);
}

void
WifiPhyStats::MacRxOkTrace (std::string context, Ptr<const Packet> packet, Ptr< Ipv4> ipv4, uint32_t interface)
{
  // NS_LOG_UNCOND("At " << Simulator::Now().GetSeconds() << " packet received with size: " << std::to_string(packet->GetSize()));
  if(packet->GetSize() > 1000){
    *streamRxOk->GetStream () << Simulator::Now().GetSeconds() << "\t" << packet->GetUid() << "\t" << packet->GetSize() << std::endl;
  }

  ++m_phyRxPkts;
  uint32_t pktSize = packet->GetSize ();
  m_phyRxBytes += pktSize;

  //NS_LOG_UNCOND ("Received PHY size=" << pktSize);
}

// void
// WifiPhyStats::IpRxDropTrace (std::string context, const Ipv4Header &header, Ptr<const Packet> packet, Ipv4L3Protocol::DropReason reason, Ptr< Ipv4> ipv4, uint32_t interface)
// {
//   // NS_LOG_UNCOND("At " << Simulator::Now().GetSeconds() << " packet received with size: " << std::to_string(packet->GetSize()));
//   if(packet->GetSize() > 1000){
//     *streamRxDrop->GetStream () << Simulator::Now().GetSeconds() << "\t" << packet->GetUid() << "\t" << packet->GetSize() << std::endl;
//   }

//   //NS_LOG_UNCOND ("Received PHY size=" << pktSize);
// }

// void
// WifiPhyStats::PhySnifferRxTrace(std::string context, Ptr< const Packet > packet, uint16_t channelFreqMhz, WifiTxVector txVector, MpduInfo aMpdu, SignalNoiseDbm signalNoise, double snr)
// {
//   // NS_LOG_UNCOND("SNR: " << std::to_string(snr));
//   *streamSNR->GetStream () << Simulator::Now().GetSeconds() << "\t" << packet->GetUid() << "\t" << snr << std::endl;
// }

uint32_t
WifiPhyStats::GetTxBytes ()
{
  return m_phyTxBytes;
}

uint32_t
WifiPhyStats::GetRxBytes ()
{
  return m_phyRxBytes;
}

void PrintPosition(Ptr<Node> node)
{
  Ptr<MobilityModel> model = node->GetObject<MobilityModel> ();
  NS_LOG_UNCOND("Position +****************************** " << model->GetPosition() << " at time " << Simulator::Now().GetSeconds());
}

// static void Rx (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet, const Address &from)
// {
// 	*stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << packet->GetSize()<< std::endl;
// }

int
main (int argc, char *argv[])
{

  RngSeedManager::SetSeed (RngSeedManager::GetRun());

  double simStopTime = 10;
  uint32_t packetSize = 1000; // in bytes
  uint32_t interPacketInterval = 8e5; // in microseconds [us]
  double distance = 10;

  CommandLine cmd;
  cmd.AddValue("simTime", "Total duration of the simulation [s])", simStopTime);
  cmd.AddValue("distance", "Distance between the two UEs [m])", distance);
  cmd.AddValue("intPck", "Time between the transmission of two packets [us])", interPacketInterval);
  cmd.AddValue("packetSize", "packet size", packetSize);
  cmd.Parse(argc, argv);

  double appRate = double(packetSize*8)/(interPacketInterval*1e-6);

  Config::SetDefault ("ns3::Ipv4L3Protocol::FragmentExpirationTimeout", TimeValue (Seconds (1)));
  // Config::SetDefault ("ns3::QueueBase::MaxPackets", UintegerValue (1000*1000));
  // Config::SetDefault ("ns3::CoDelQueueDisc::Mode", StringValue ("QUEUE_DISC_MODE_PACKETS"));
  // Config::SetDefault ("ns3::CoDelQueueDisc::MaxPackets", UintegerValue (50000));

  // This is set to communicate with a range longer than 100 meters
  // Config::SetDefault ("ns3::WifiPhy::TxPowerStart", DoubleValue (33));
  // Config::SetDefault ("ns3::WifiPhy::TxPowerEnd", DoubleValue (33));
  std::string phyMode ("OfdmRate6MbpsBW10MHz");
  // The below set of helpers will help us to put together the wifi NICs we want
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();
  wifiPhy.SetChannel (channel);
  // wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel");
  // ns-3 supports generate a pcap trace
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);
  NqosMmWaveWaveMacHelper wifi80211pMac = NqosMmWaveWaveMacHelper::Default ();
  MmWaveWifi80211pHelper wifi80211p = MmWaveWifi80211pHelper::Default ();

  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
    "DataMode",StringValue (phyMode),
    "ControlMode",StringValue (phyMode));

  Ptr < Building > building = Create<Building> ();
  building->SetBoundaries (Box (60,110,
    10, 15,
    0, 30));

  NodeContainer ueNodes;
  ueNodes.Create (2);

  NetDeviceContainer devices = wifi80211p.Install (wifiPhy, wifi80211pMac, ueNodes);
  wifiPhy.EnablePcap ("wave-simple-80211p", devices);

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 1.5));
  positionAlloc->Add (Vector (distance, 0.0, 1.5));

  MobilityHelper mobility;
  mobility.SetPositionAllocator(positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install (ueNodes);
  // BuildingsHelper::Install(ueNodes);

  ueNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, 0, 0));
  // ueNodes.Get (1)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (50, 0, 0));

  ueNodes.Get (1)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (0, 0, 0));
	// Install the IP stack on the UEs
	// Assign IP address to UEs, and install applications
  InternetStackHelper internet;
  internet.Install (ueNodes);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  // Install and start applications on UEs and remote host
  uint16_t dlPort = 1234;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  // DL UDP
  UdpServerHelper clientHelper (dlPort);
  clientApps.Add (clientHelper.Install (ueNodes.Get(0)));

  UdpClientHelper serverHelper (i.GetAddress (0), dlPort); // Destination address
  serverHelper.SetAttribute ("Interval", TimeValue (MicroSeconds(interPacketInterval)));
  serverHelper.SetAttribute ("PacketSize", UintegerValue(packetSize));
  serverHelper.SetAttribute ("MaxPackets", UintegerValue(10000));
  serverApps.Add (serverHelper.Install (ueNodes.Get(1)));

  // wifiPhy->TraceConnectWithoutContext("RxOk", MakeBoundCallback (&WifiPhyStats::PhyRxTrace, m_wifiPhyStats));
  clientApps.Start (Seconds (0));
  clientApps.Stop (Seconds (simStopTime));
  serverApps.Start (Seconds (0.1));
  serverApps.Stop (Seconds (simStopTime));

  // BuildingsHelper::MakeMobilityModelConsistent();

  std::ostringstream RxfileName;
  RxfileName << "WaveResults/UDP-DATA-wave-RxOk_" << RngSeedManager::GetRun() << "_" << distance << "_" << appRate <<  ".txt";

  std::ostringstream TxfileName;
  TxfileName << "WaveResults/UDP-DATA-wave-Tx_" << RngSeedManager::GetRun() << "_" << distance << "_" << appRate <<  ".txt";

  std::ostringstream SNRfileName;
  SNRfileName << "WaveResults/UDP-DATA-wave-SNR_" << RngSeedManager::GetRun() << "_" << distance << "_" << appRate <<  ".txt";

  // std::ostringstream RxDropfileName;
  // RxDropfileName << "WaveResults/UDP-DATA-wave-RxDrop_" << RngSeedManager::GetRun() << "_" << distance << "_" << appRate <<  ".txt";

  AsciiTraceHelper asciiTraceHelper;

  streamRxOk = asciiTraceHelper.CreateFileStream (RxfileName.str ().c_str ());
  streamTx = asciiTraceHelper.CreateFileStream (TxfileName.str ().c_str ());
  streamSNR = asciiTraceHelper.CreateFileStream (SNRfileName.str ().c_str ());
  // streamRxDrop = asciiTraceHelper.CreateFileStream (RxDropfileName.str ().c_str ());

  Ptr<WifiPhyStats> m_wifiPhyStats = CreateObject<WifiPhyStats> (); ///< wifi phy statistics
  Config::Connect("/NodeList/1/$ns3::Ipv4L3Protocol/Tx", MakeCallback (&WifiPhyStats::MacTxTrace, m_wifiPhyStats));
  Config::Connect ("/NodeList/0/$ns3::Ipv4L3Protocol/Rx", MakeCallback (&WifiPhyStats::MacRxOkTrace,  m_wifiPhyStats));
  // Config::Connect ("/NodeList/*/DeviceList/*/Mac/MacTx", MakeCallback (&WifiPhyStats::MacTxTrace, m_wifiPhyStats));
  // Config::Connect ("/NodeList/*/$ns3::Ipv4L3Protocol/Drop", MakeCallback (&WifiPhyStats::IpRxDropTrace,  m_wifiPhyStats));
  // Config::Connect ("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Phy/MonitorSnifferRx", MakeCallback (&WifiPhyStats::PhySnifferRxTrace,  m_wifiPhyStats));

	Config::Set ("/NodeList/*/DeviceList/*/TxQueue/MaxPackets", UintegerValue (1000*1000));
	Config::Set ("/NodeList/*/DeviceList/*/TxQueue/MaxBytes", UintegerValue (1500*1000*1000));

  Simulator::Stop (Seconds (simStopTime));
  Simulator::Run();
  Simulator::Destroy ();

  uint32_t totalTxPhyBytes = m_wifiPhyStats->GetTxBytes ();
  uint32_t totalRxPhyBytes = m_wifiPhyStats->GetRxBytes ();

  NS_LOG_UNCOND("Bytes sent : " << std::to_string(totalTxPhyBytes) << "\t Bytes received: " << std::to_string(totalRxPhyBytes));
  NS_LOG_UNCOND("Bit Error Rate : " << std::to_string( 1 - double(totalRxPhyBytes)/totalTxPhyBytes));
  double PER = 1 - double(totalRxPhyBytes)/totalTxPhyBytes;

  std::ostringstream PERfileName;
  PERfileName << "WaveResults/UDP-DATA-wave-PER_" << distance << "_" << appRate << ".txt";
  streamPER = asciiTraceHelper.CreateFileStream (PERfileName.str ().c_str (), std::ios::app);

  *streamPER->GetStream () << RngSeedManager::GetRun() << "\t" << PER << std::endl;

  return 0;

}
