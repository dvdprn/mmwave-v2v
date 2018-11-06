/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Junling Bu <linlinjavaer@gmail.com>
 */

#include "mmwave-wave-channel-scheduler.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveWaveChannelScheduler");

NS_OBJECT_ENSURE_REGISTERED (MmWaveWaveChannelScheduler);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
TypeId
MmWaveWaveChannelScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveWaveChannelScheduler")
    .SetParent<Object> ()
    .SetGroupName ("MmWave-Wave")
  ;
  return tid;
}

MmWaveWaveChannelScheduler::MmWaveWaveChannelScheduler ()
{
  NS_LOG_FUNCTION (this);
}
MmWaveWaveChannelScheduler::~MmWaveWaveChannelScheduler ()
{
  NS_LOG_FUNCTION (this);
}

void
MmWaveWaveChannelScheduler::DoInitialize (void)
{
  // assign default CCH access when the device is initialized
  AssignDefaultCchAccess ();
}

void
MmWaveWaveChannelScheduler::SetMmWaveWaveNetDevice (Ptr<MmWaveWaveNetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  m_device = device;
}

bool
MmWaveWaveChannelScheduler::IsChannelAccessAssigned (uint32_t channelNumber) const
{
  NS_LOG_FUNCTION (this << channelNumber);
  return (GetAssignedAccessType (channelNumber) != NoAccess);
}

bool
MmWaveWaveChannelScheduler::IsCchAccessAssigned (void) const
{
  NS_LOG_FUNCTION (this);
  return (GetAssignedAccessType (CCH) != NoAccess);
}

bool
MmWaveWaveChannelScheduler::IsSchAccessAssigned (void) const
{
  NS_LOG_FUNCTION (this);
  return (GetAssignedAccessType (SCH1) != NoAccess) || (GetAssignedAccessType (SCH2) != NoAccess)
         || (GetAssignedAccessType (SCH3) != NoAccess) || (GetAssignedAccessType (SCH4) != NoAccess)
         || (GetAssignedAccessType (SCH5) != NoAccess) || (GetAssignedAccessType (SCH6) != NoAccess);
}

bool
MmWaveWaveChannelScheduler::IsContinuousAccessAssigned (uint32_t channelNumber) const
{
  NS_LOG_FUNCTION (this << channelNumber);
  return (GetAssignedAccessType (channelNumber) == ContinuousAccess);
}
bool
MmWaveWaveChannelScheduler::IsAlternatingAccessAssigned (uint32_t channelNumber) const
{
  NS_LOG_FUNCTION (this << channelNumber);
  return (GetAssignedAccessType (channelNumber) == AlternatingAccess);
}
bool
MmWaveWaveChannelScheduler::IsExtendedAccessAssigned (uint32_t channelNumber) const
{
  NS_LOG_FUNCTION (this << channelNumber);
  return (GetAssignedAccessType (channelNumber) == ExtendedAccess);
}
bool
MmWaveWaveChannelScheduler::IsDefaultCchAccessAssigned (void) const
{
  NS_LOG_FUNCTION (this);
  return (GetAssignedAccessType (CCH) == DefaultCchAccess);
}
bool
MmWaveWaveChannelScheduler::StartSch (const SchInfo & schInfo)
{
  NS_LOG_FUNCTION (this << &schInfo);
  uint32_t cn = schInfo.channelNumber;

  if (MmWaveWaveChannelManager::IsCch (schInfo.channelNumber))
    {
      NS_LOG_DEBUG ("the channel access requirement for CCH is not allowed.");
      return false;
    }
  uint32_t extends = schInfo.extendedAccess;
  bool immediate = schInfo.immediateAccess;
  Ptr<MmWaveWaveOcbWifiMac> mac = m_device->GetMac (cn);
  for (EdcaParametersI i = schInfo.edcaParameters.begin (); i != schInfo.edcaParameters.end (); ++i)
    {
      EdcaParameter edca = i->second;
      mac->ConfigureEdca (edca.cwmin, edca.cwmax, edca.aifsn, i->first);
    }

  if (extends == EXTENDED_CONTINUOUS)
    {
      return AssignContinuousAccess (cn, immediate);
    }
  else if (extends == EXTENDED_ALTERNATING)
    {
      return AssignAlternatingAccess (cn, immediate);
    }
  else
    {
      return AssignExtendedAccess (cn, extends, immediate);
    }
}

bool
MmWaveWaveChannelScheduler::StopSch (uint32_t channelNumber)
{
  NS_LOG_FUNCTION (this << channelNumber);
  if (MmWaveWaveChannelManager::IsCch (channelNumber))
    {
      NS_LOG_DEBUG ("the channel access for CCH is not allowed to be released.");
      return false;
    }
  if (!IsChannelAccessAssigned (channelNumber))
    {
      NS_LOG_DEBUG ("the channel access for channel " << channelNumber << " has already been released.");
      return true;
    }
  return ReleaseAccess (channelNumber);
}

} // namespace ns3
