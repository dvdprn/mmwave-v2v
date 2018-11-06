/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
 *         Junling Bu <linlinjavaer@gmail.com>
 */
#include <algorithm>
#include "ns3/node.h"
#include "ns3/wifi-phy.h"
#include "ns3/llc-snap-header.h"
#include "ns3/channel.h"
#include "ns3/log.h"
#include "ns3/socket.h"
#include "ns3/object-map.h"
#include "ns3/object-vector.h"
#include "mmwave-wave-net-device.h"
#include "mmwave-wave-higher-tx-tag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveWaveNetDevice");

NS_OBJECT_ENSURE_REGISTERED (MmWaveWaveNetDevice);

TypeId
MmWaveWaveNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveWaveNetDevice")
    .SetParent<NetDevice> ()
    .SetGroupName ("MmWave-Wave")
    .AddConstructor<MmWaveWaveNetDevice> ()
    .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                   UintegerValue (MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH),
                   MakeUintegerAccessor (&MmWaveWaveNetDevice::SetMtu,
                                         &MmWaveWaveNetDevice::GetMtu),
                   MakeUintegerChecker<uint16_t> (1,MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH))
    .AddAttribute ("Channel", "The channel attached to this device",
                   PointerValue (),
                   MakePointerAccessor (&MmWaveWaveNetDevice::GetChannel),
                   MakePointerChecker<Channel> ())
    .AddAttribute ("PhyEntities", "The PHY entities attached to this device.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&MmWaveWaveNetDevice::m_phyEntities),
                   MakeObjectVectorChecker<WifiPhy> ())
    .AddAttribute ("MacEntities", "The MAC layer attached to this device.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&MmWaveWaveNetDevice::m_macEntities),
                   MakeObjectMapChecker<MmWaveWaveOcbWifiMac> ())
    .AddAttribute ("ChannelScheduler", "The channel scheduler attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&MmWaveWaveNetDevice::SetMmWaveWaveChannelScheduler,
                                        &MmWaveWaveNetDevice::GetMmWaveWaveChannelScheduler),
                   MakePointerChecker<MmWaveWaveChannelScheduler> ())
    .AddAttribute ("ChannelManager", "The channel manager attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&MmWaveWaveNetDevice::SetMmWaveWaveChannelManager,
                                        &MmWaveWaveNetDevice::GetMmWaveWaveChannelManager),
                   MakePointerChecker<MmWaveWaveChannelManager> ())
    .AddAttribute ("ChannelCoordinator", "The channel coordinator attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&MmWaveWaveNetDevice::SetMmWaveWaveChannelCoordinator,
                                        &MmWaveWaveNetDevice::GetMmWaveWaveChannelCoordinator),
                   MakePointerChecker<MmWaveWaveChannelCoordinator> ())
    .AddAttribute ("VsaManager", "The VSA manager attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&MmWaveWaveNetDevice::SetMmWaveWaveVsaManager,
                                        &MmWaveWaveNetDevice::GetMmWaveWaveVsaManager),
                   MakePointerChecker<MmWaveWaveVsaManager> ())
  ;
  return tid;
}

MmWaveWaveNetDevice::MmWaveWaveNetDevice (void)
  : m_txProfile (0)
{
  NS_LOG_FUNCTION (this);
}

MmWaveWaveNetDevice::~MmWaveWaveNetDevice (void)
{
  NS_LOG_FUNCTION (this);
}

void
MmWaveWaveNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  if (m_txProfile != 0)
    {
      delete m_txProfile;
      m_txProfile = 0;
    }
  for (PhyEntitiesI i = m_phyEntities.begin (); i != m_phyEntities.end (); ++i)
    {
      Ptr<WifiPhy> phy = (*i);
      phy->Dispose ();
    }
  m_phyEntities.clear ();
  for (MacEntitiesI i = m_macEntities.begin (); i != m_macEntities.end (); ++i)
    {
      Ptr<MmWaveWaveOcbWifiMac> mac = i->second;
      Ptr<WifiRemoteStationManager> stationManager = mac->GetWifiRemoteStationManager ();
      stationManager->Dispose ();
      mac->Dispose ();
    }
  m_macEntities.clear ();
  m_channelCoordinator->Dispose ();
  m_channelManager->Dispose ();
  m_channelScheduler->Dispose ();
  m_vsaManager->Dispose ();
  m_channelCoordinator = 0;
  m_channelManager = 0;
  m_channelScheduler = 0;
  m_vsaManager = 0;
  // chain up.
  NetDevice::DoDispose ();
}

void
MmWaveWaveNetDevice::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  if (m_phyEntities.size () == 0)
    {
      NS_FATAL_ERROR ("there is no PHY entity in this WAVE device");
    }
  for (PhyEntitiesI i = m_phyEntities.begin (); i != m_phyEntities.end (); ++i)
    {
      Ptr<WifiPhy> phy = (*i);
      phy->Initialize ();
    }
  if (m_macEntities.size () == 0)
    {
      NS_FATAL_ERROR ("there is no MAC entity in this WAVE device");
    }
  for (MacEntitiesI i = m_macEntities.begin (); i != m_macEntities.end (); ++i)
    {
      Ptr<MmWaveWaveOcbWifiMac> mac = i->second;
      mac->SetForwardUpCallback (MakeCallback (&MmWaveWaveNetDevice::ForwardUp, this));
      // Make each MAC entity in sleep mode.
      mac->Suspend ();
      mac->Initialize ();

      Ptr<WifiRemoteStationManager> stationManager = mac->GetWifiRemoteStationManager ();
      // Currently PHY is not attached to MAC and will be dynamically attached and unattached to MAC latter,
      // however WifiRemoteStationManager in the MAC shall know something  in the PHY such as supported data rates.
      // Since these information can be treated as same when same PHY devices are added, here we force
      // all of WifiRemoteStationManagers in multiple MAC entities only associate with single PHY device even there may
      // be multiple PHY entities. This approach may be strange but can work fine.
      stationManager->SetupPhy (m_phyEntities[0]);
      stationManager->Initialize ();
    }
  m_channelScheduler->SetMmWaveWaveNetDevice (this);
  m_vsaManager->SetMmWaveWaveNetDevice (this);
  m_channelScheduler->Initialize ();
  m_channelCoordinator->Initialize ();
  m_channelManager->Initialize ();
  m_vsaManager->Initialize ();
  NetDevice::DoInitialize ();
}

void
MmWaveWaveNetDevice::AddMac (uint32_t channelNumber, Ptr<MmWaveWaveOcbWifiMac> mac)
{
  NS_LOG_FUNCTION (this << channelNumber << mac);
  if (!MmWaveWaveChannelManager::IsWaveChannel (channelNumber))
    {
      NS_FATAL_ERROR ("The channel " << channelNumber << " is not a valid WAVE channel number");
    }
  if (m_macEntities.find (channelNumber) != m_macEntities.end ())
    {
      NS_FATAL_ERROR ("The MAC entity for channel " << channelNumber << " already exists.");
    }
  m_macEntities.insert (std::make_pair (channelNumber, mac));
}
Ptr<MmWaveWaveOcbWifiMac>
MmWaveWaveNetDevice::GetMac (uint32_t channelNumber) const
{
  NS_LOG_FUNCTION (this << channelNumber);
  MacEntitiesI i = m_macEntities.find (channelNumber);
  if (i == m_macEntities.end ())
    {
      NS_FATAL_ERROR ("there is no available MAC entity for channel " << channelNumber);
    }
  return i->second;
}

std::map<uint32_t, Ptr<MmWaveWaveOcbWifiMac> >
MmWaveWaveNetDevice::GetMacs (void) const
{
  NS_LOG_FUNCTION (this);
  return m_macEntities;
}

void
MmWaveWaveNetDevice::AddPhy (Ptr<WifiPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  if (std::find (m_phyEntities.begin (), m_phyEntities.end (), phy) != m_phyEntities.end ())
    {
      NS_FATAL_ERROR ("This PHY entity is already inserted");
    }
  m_phyEntities.push_back (phy);
}
Ptr<WifiPhy>
MmWaveWaveNetDevice::GetPhy (uint32_t index) const
{
  NS_LOG_FUNCTION (this << index);
  return m_phyEntities.at (index);
}

std::vector<Ptr<WifiPhy> >
MmWaveWaveNetDevice::GetPhys (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phyEntities;
}

bool
MmWaveWaveNetDevice::StartVsa (const VsaInfo & vsaInfo)
{
  NS_LOG_FUNCTION (this << &vsaInfo);
  if (!IsAvailableChannel ( vsaInfo.channelNumber))
    {
      return false;
    }
  if (!m_channelScheduler->IsChannelAccessAssigned (vsaInfo.channelNumber))
    {
      NS_LOG_DEBUG ("there is no channel access assigned for channel " << vsaInfo.channelNumber);
      return false;
    }
  if (vsaInfo.vsc == 0)
    {
      NS_LOG_DEBUG ("vendor specific information shall not be null");
      return false;
    }
  if (vsaInfo.oi.IsNull () && vsaInfo.managementId >= 16)
    {
      NS_LOG_DEBUG ("when organization identifier is not set, management ID "
                    "shall be in range from 0 to 15");
      return false;
    }

  m_vsaManager->SendVsa (vsaInfo);
  return true;
}


bool
MmWaveWaveNetDevice::StopVsa (uint32_t channelNumber)
{
  NS_LOG_FUNCTION (this << channelNumber);
  if (!IsAvailableChannel (channelNumber))
    {
      return false;
    }
  m_vsaManager->RemoveByChannel (channelNumber);
  return true;
}

void
MmWaveWaveNetDevice::SetWaveVsaCallback (WaveVsaCallback vsaCallback)
{
  NS_LOG_FUNCTION (this);
  m_vsaManager->SetWaveVsaCallback (vsaCallback);
}

bool
MmWaveWaveNetDevice::StartSch (const SchInfo & schInfo)
{
  NS_LOG_FUNCTION (this << &schInfo);
  if (!IsAvailableChannel (schInfo.channelNumber))
    {
      return false;
    }
  return m_channelScheduler->StartSch (schInfo);
}

bool
MmWaveWaveNetDevice::StopSch (uint32_t channelNumber)
{
  NS_LOG_FUNCTION (this << channelNumber);
  if (!IsAvailableChannel (channelNumber))
    {
      return false;
    }
  return m_channelScheduler->StopSch (channelNumber);
}

bool
MmWaveWaveNetDevice::RegisterTxProfile (const TxProfile & txprofile)
{
  NS_LOG_FUNCTION (this << &txprofile);
  if (m_txProfile != 0)
    {
      return false;
    }
  if (!IsAvailableChannel (txprofile.channelNumber))
    {
      return false;
    }
  if (txprofile.txPowerLevel > 8)
    {
      return false;
    }
  // IP-based packets is not allowed to send in the CCH.
  if (txprofile.channelNumber == CCH)
    {
      NS_LOG_DEBUG ("IP-based packets shall not be transmitted on the CCH");
      return false;
    }
  if  (txprofile.dataRate == WifiMode () || txprofile.txPowerLevel == 8)
    {
      // let MAC layer itself determine tx parameters.
      NS_LOG_DEBUG ("High layer does not want to control tx parameters.");
    }
  else
    {
      // if current PHY devices do not support data rate of the tx profile
      for (PhyEntitiesI i = m_phyEntities.begin (); i != m_phyEntities.end (); ++i)
        {
          if (!((*i)->IsModeSupported (txprofile.dataRate)))
            {
              NS_LOG_DEBUG ("This data rate " << txprofile.dataRate.GetUniqueName () << " is not supported by current PHY device");
              return false;
            }
        }
    }

  m_txProfile = new TxProfile ();
  *m_txProfile = txprofile;
  return true;
}

bool
MmWaveWaveNetDevice::DeleteTxProfile (uint32_t channelNumber)
{
  NS_LOG_FUNCTION (this << channelNumber);
  if (!IsAvailableChannel (channelNumber))
    {
      return false;
    }
  if (m_txProfile == 0)
    {
      return false;
    }
  if (m_txProfile->channelNumber != channelNumber)
    {
      return false;
    }

  delete m_txProfile;
  m_txProfile = 0;
  return true;
}

bool
MmWaveWaveNetDevice::SendX (Ptr<Packet> packet, const Address & dest, uint32_t protocol, const TxInfo & txInfo)
{
  NS_LOG_FUNCTION (this << packet << dest << protocol << &txInfo);
  if (!IsAvailableChannel (txInfo.channelNumber))
    {
      return false;
    }
  if (!m_channelScheduler->IsChannelAccessAssigned (txInfo.channelNumber))
    {
      NS_LOG_DEBUG ("there is no channel access assigned for channel " << txInfo.channelNumber);
      return false;
    }
  if ((txInfo.channelNumber == CCH) && (protocol == IPv4_PROT_NUMBER || protocol == IPv6_PROT_NUMBER))
    {
      NS_LOG_DEBUG ("IP-based packets shall not be transmitted on the CCH");
      return false;
    }
  if ((txInfo.priority > 7) || txInfo.txPowerLevel > 8)
    {
      NS_LOG_DEBUG ("invalid transmit parameters.");
      return false;
    }

  if ((txInfo.dataRate == WifiMode ()) ||  (txInfo.txPowerLevel == 8))
    {
      // let MAC layer itself determine tx parameters.
      NS_LOG_DEBUG ("High layer does not want to control tx parameters.");
    }
  else
    {
      // if current PHY devices do not support data rate of the  tx profile
      for (PhyEntitiesI i = m_phyEntities.begin (); i != m_phyEntities.end (); ++i)
        {
          if ( !((*i)->IsModeSupported (txInfo.dataRate)))
            {
              return false;
            }
        }
      WifiTxVector txVector;
      txVector.SetChannelWidth (10);
      txVector.SetTxPowerLevel (txInfo.txPowerLevel);
      txVector.SetMode (txInfo.dataRate);
      txVector.SetPreambleType (txInfo.preamble);
      MmWaveWaveHigherLayerTxVectorTag tag = MmWaveWaveHigherLayerTxVectorTag (txVector, false);
      packet->AddPacketTag (tag);
    }

  LlcSnapHeader llc;
  llc.SetType (protocol);
  packet->AddHeader (llc);

  // according to channel number and priority,
  // route the packet to a proper queue.
  SocketPriorityTag prio;
  prio.SetPriority (txInfo.priority);
  packet->ReplacePacketTag (prio);
  Ptr<WifiMac> mac = GetMac (txInfo.channelNumber);
  Mac48Address realTo = Mac48Address::ConvertFrom (dest);
  mac->NotifyTx (packet);
  mac->Enqueue (packet, realTo);
  return true;
}

void
MmWaveWaveNetDevice::ChangeAddress (Address newAddress)
{
  NS_LOG_FUNCTION (this << newAddress);
  Address oldAddress = GetAddress ();
  if (newAddress == oldAddress)
    {
      return;
    }
  SetAddress (newAddress);
  // Since MAC address is changed, the MAC layer including multiple MAC entities should be reset
  // and internal MAC queues will be flushed.
  for (MacEntitiesI i = m_macEntities.begin (); i != m_macEntities.end (); ++i)
    {
      i->second->Reset ();
    }
  m_addressChange (oldAddress, newAddress);
}

void
MmWaveWaveNetDevice::CancelTx (uint32_t channelNumber, enum AcIndex ac)
{
  if (IsAvailableChannel (channelNumber))
    {
      return;
    }
  Ptr<MmWaveWaveOcbWifiMac> mac = GetMac (channelNumber);
  mac->CancleTx (ac);
}

void
MmWaveWaveNetDevice::SetMmWaveWaveChannelManager (Ptr<MmWaveWaveChannelManager> channelManager)
{
  m_channelManager = channelManager;
}
Ptr<MmWaveWaveChannelManager>
MmWaveWaveNetDevice::GetMmWaveWaveChannelManager (void) const
{
  return m_channelManager;
}
void
MmWaveWaveNetDevice::SetMmWaveWaveChannelScheduler (Ptr<MmWaveWaveChannelScheduler> channelScheduler)
{
  m_channelScheduler = channelScheduler;
}
Ptr<MmWaveWaveChannelScheduler>
MmWaveWaveNetDevice::GetMmWaveWaveChannelScheduler (void) const
{
  return m_channelScheduler;
}
void
MmWaveWaveNetDevice::SetMmWaveWaveChannelCoordinator (Ptr<MmWaveWaveChannelCoordinator> channelCoordinator)
{
  m_channelCoordinator = channelCoordinator;
}
Ptr<MmWaveWaveChannelCoordinator>
MmWaveWaveNetDevice::GetMmWaveWaveChannelCoordinator (void) const
{
  return m_channelCoordinator;
}
void
MmWaveWaveNetDevice::SetMmWaveWaveVsaManager (Ptr<MmWaveWaveVsaManager> vsaManager)
{
  m_vsaManager = vsaManager;
}
Ptr<MmWaveWaveVsaManager>
MmWaveWaveNetDevice::GetMmWaveWaveVsaManager (void) const
{
  return m_vsaManager;
}

void
MmWaveWaveNetDevice::SetIfIndex (const uint32_t index)
{
  m_ifIndex = index;
}
uint32_t
MmWaveWaveNetDevice::GetIfIndex (void) const
{
  return m_ifIndex;
}
Ptr<Channel>
MmWaveWaveNetDevice::GetChannel (void) const
{
  NS_ASSERT (!m_phyEntities.empty ());
  return GetPhy (0)->GetChannel ();
}
void
MmWaveWaveNetDevice::SetAddress (Address address)
{
  for (MacEntitiesI i = m_macEntities.begin (); i != m_macEntities.end (); ++i)
    {
      i->second->SetAddress (Mac48Address::ConvertFrom (address));
    }
}
Address
MmWaveWaveNetDevice::GetAddress (void) const
{
  return (GetMac (CCH))->GetAddress ();
}
bool
MmWaveWaveNetDevice::SetMtu (const uint16_t mtu)
{
  if (mtu > MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH)
    {
      return false;
    }
  m_mtu = mtu;
  return true;
}
uint16_t
MmWaveWaveNetDevice::GetMtu (void) const
{
  return m_mtu;
}
bool
MmWaveWaveNetDevice::IsLinkUp (void) const
{
  // Different from WifiNetDevice::IsLinkUp, a MmWaveWaveNetDevice device
  // is always link up so the m_linkup variable is true forever.
  // Even the device is in channel switch state, packets can still be queued.
  return true;
}
void
MmWaveWaveNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_WARN ("MmWaveWaveNetDevice is linkup forever, so this callback will be never called");
}
bool
MmWaveWaveNetDevice::IsBroadcast (void) const
{
  return true;
}
Address
MmWaveWaveNetDevice::GetBroadcast (void) const
{
  return Mac48Address::GetBroadcast ();
}
bool
MmWaveWaveNetDevice::IsMulticast (void) const
{
  return true;
}
Address
MmWaveWaveNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  return Mac48Address::GetMulticast (multicastGroup);
}
Address MmWaveWaveNetDevice::GetMulticast (Ipv6Address addr) const
{
  return Mac48Address::GetMulticast (addr);
}
bool
MmWaveWaveNetDevice::IsPointToPoint (void) const
{
  return false;
}
bool
MmWaveWaveNetDevice::IsBridge (void) const
{
  return false;
}

bool
MmWaveWaveNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocol)
{
  NS_LOG_FUNCTION (this << packet << dest << protocol);
  if (m_txProfile == 0)
    {
      NS_LOG_DEBUG ("there is no tx profile registered for transmission");
      return false;
    }
  if (!m_channelScheduler->IsChannelAccessAssigned (m_txProfile->channelNumber))
    {
      NS_LOG_DEBUG ("there is no channel access assigned for channel " << m_txProfile->channelNumber);
      return false;
    }
  if (m_txProfile->dataRate == WifiMode () || m_txProfile->txPowerLevel == 8)
    {
      // let MAC layer itself determine tx parameters.
      NS_LOG_DEBUG ("High layer does not want to control tx parameters.");
    }
  else
    {
      WifiTxVector txVector;
      txVector.SetTxPowerLevel (m_txProfile->txPowerLevel);
      txVector.SetMode (m_txProfile->dataRate);
      txVector.SetPreambleType (m_txProfile->preamble);
      MmWaveWaveHigherLayerTxVectorTag tag = MmWaveWaveHigherLayerTxVectorTag (txVector, m_txProfile->adaptable);
      packet->AddPacketTag (tag);
    }

  LlcSnapHeader llc;
  llc.SetType (protocol);
  packet->AddHeader (llc);

  // qos tag is already inserted into the packet by high layer  or with default value 7 if high layer forget it.
  Ptr<WifiMac> mac = GetMac (m_txProfile->channelNumber);
  Mac48Address realTo = Mac48Address::ConvertFrom (dest);
  mac->NotifyTx (packet);
  mac->Enqueue (packet, realTo);
  return true;
}

Ptr<Node>
MmWaveWaveNetDevice::GetNode (void) const
{
  return m_node;
}
void
MmWaveWaveNetDevice::SetNode (Ptr<Node> node)
{
  m_node = node;
}
bool
MmWaveWaveNetDevice::NeedsArp (void) const
{
  // Whether NeedsArp or not?
  // For IP-based packets , yes; For WSMP packets, no;
  // so return true always.
  return true;
}
void
MmWaveWaveNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  m_forwardUp = cb;
}

bool
MmWaveWaveNetDevice::IsAvailableChannel (uint32_t channelNumber) const
{
  if (!MmWaveWaveChannelManager::IsWaveChannel (channelNumber))
    {
      NS_LOG_DEBUG ("this is no a valid WAVE channel for channel " << channelNumber);
      return false;
    }
  if (m_macEntities.find (channelNumber) == m_macEntities.end ())
    {
      NS_LOG_DEBUG ("this is no available WAVE entity  for channel " << channelNumber);
      return false;
    }
  return true;
}

void
MmWaveWaveNetDevice::ForwardUp (Ptr<Packet> packet, Mac48Address from, Mac48Address to)
{
  NS_LOG_FUNCTION (this << packet << from << to);
  LlcSnapHeader llc;
  packet->RemoveHeader (llc);
  enum NetDevice::PacketType type;
  if (to.IsBroadcast ())
    {
      type = NetDevice::PACKET_BROADCAST;
    }
  else if (to.IsGroup ())
    {
      type = NetDevice::PACKET_MULTICAST;
    }
  else if (to == GetAddress ())
    {
      type = NetDevice::PACKET_HOST;
    }
  else
    {
      type = NetDevice::PACKET_OTHERHOST;
    }

  if (type != NetDevice::PACKET_OTHERHOST)
    {
      // currently we cannot know from which MAC entity the packet is received,
      // so we use the MAC entity for CCH as it receives this packet.
      Ptr<MmWaveWaveOcbWifiMac> mac = GetMac (CCH);
      mac->NotifyRx (packet);
      m_forwardUp (this, packet, llc.GetType (), from);
    }

  if (!m_promiscRx.IsNull ())
    {
      // currently we cannot know from which MAC entity the packet is received,
      // so we use the MAC entity for CCH as it receives this packet.
      Ptr<MmWaveWaveOcbWifiMac> mac = GetMac (CCH);
      mac->NotifyPromiscRx (packet);
      m_promiscRx (this, packet, llc.GetType (), from, to, type);
    }
}

bool
MmWaveWaveNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocol)
{
  NS_LOG_FUNCTION (this << packet << source << dest << protocol);
  return false;
}

void
MmWaveWaveNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  m_promiscRx = cb;
  for (MacEntitiesI i = m_macEntities.begin (); i != m_macEntities.end (); ++i)
    {
      i->second->SetPromisc ();
    }
}

bool
MmWaveWaveNetDevice::SupportsSendFrom (void) const
{
  return (GetMac (CCH))->SupportsSendFrom ();
}

} // namespace ns3
