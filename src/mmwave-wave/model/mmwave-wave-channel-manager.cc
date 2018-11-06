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
#include "mmwave-wave-channel-manager.h"
#include "ns3/log.h"
#include "ns3/assert.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveWaveChannelManager");

NS_OBJECT_ENSURE_REGISTERED (MmWaveWaveChannelManager);

TypeId
MmWaveWaveChannelManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveWaveChannelManager")
    .SetParent<Object> ()
    .SetGroupName ("MmWave-Wave")
    .AddConstructor<MmWaveWaveChannelManager> ()
  ;
  return tid;
}

MmWaveWaveChannelManager::MmWaveWaveChannelManager ()
{
  NS_LOG_FUNCTION (this);
  m_channels.insert (std::make_pair (CCH, new WaveChannel (CCH)));
  m_channels.insert (std::make_pair (SCH1, new WaveChannel (SCH1)));
  m_channels.insert (std::make_pair (SCH2, new WaveChannel (SCH2)));
  m_channels.insert (std::make_pair (SCH3, new WaveChannel (SCH3)));
  m_channels.insert (std::make_pair (SCH4, new WaveChannel (SCH4)));
  m_channels.insert (std::make_pair (SCH5, new WaveChannel (SCH5)));
  m_channels.insert (std::make_pair (SCH6, new WaveChannel (SCH6)));
}

MmWaveWaveChannelManager::~MmWaveWaveChannelManager ()
{
  NS_LOG_FUNCTION (this);
  std::map<uint32_t, WaveChannel *> ::iterator i;
  for (i = m_channels.begin (); i != m_channels.end (); ++i)
    {
      delete (i->second);
    }
  m_channels.clear ();
}

uint32_t
MmWaveWaveChannelManager::GetCch (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return CCH;
}

std::vector<uint32_t>
MmWaveWaveChannelManager::GetSchs (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  std::vector<uint32_t> schs;
  schs.push_back (SCH1);
  schs.push_back (SCH2);
  schs.push_back (SCH3);
  schs.push_back (SCH4);
  schs.push_back (SCH5);
  schs.push_back (SCH6);
  return schs;
}

std::vector<uint32_t>
MmWaveWaveChannelManager::GetWaveChannels (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  std::vector<uint32_t> channels;
  channels.push_back (CCH);
  channels.push_back (SCH1);
  channels.push_back (SCH2);
  channels.push_back (SCH3);
  channels.push_back (SCH4);
  channels.push_back (SCH5);
  channels.push_back (SCH6);
  return channels;
}

uint32_t
MmWaveWaveChannelManager::GetNumberOfWaveChannels (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  static  uint32_t NumberOfWaveChannels  = GetWaveChannels ().size ();
  return NumberOfWaveChannels;
}

bool
MmWaveWaveChannelManager::IsCch (uint32_t channelNumber)
{
  NS_LOG_FUNCTION_NOARGS ();
  return channelNumber == CCH;
}

bool
MmWaveWaveChannelManager::IsSch (uint32_t channelNumber)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (channelNumber < SCH1 || channelNumber > SCH6)
    {
      return false;
    }
  if (channelNumber % 2 == 1)
    {
      return false;
    }
  return (channelNumber != CCH);
}

bool
MmWaveWaveChannelManager::IsWaveChannel (uint32_t channelNumber)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (channelNumber < SCH1 || channelNumber > SCH6)
    {
      return false;
    }
  if (channelNumber % 2 == 1)
    {
      return false;
    }
  return true;
}

uint32_t
MmWaveWaveChannelManager::GetOperatingClass (uint32_t channelNumber)
{
  NS_LOG_FUNCTION (this << channelNumber);
  return m_channels[channelNumber]->operatingClass;
}

bool
MmWaveWaveChannelManager::GetManagementAdaptable (uint32_t channelNumber)
{
  NS_LOG_FUNCTION (this << channelNumber);
  return m_channels[channelNumber]->adaptable;
}

WifiMode
MmWaveWaveChannelManager::GetManagementDataRate (uint32_t channelNumber)
{
  NS_LOG_FUNCTION (this << channelNumber);
  return m_channels[channelNumber]->dataRate;
}

WifiPreamble
MmWaveWaveChannelManager::GetManagementPreamble (uint32_t channelNumber)
{
  NS_LOG_FUNCTION (this << channelNumber);
  return m_channels[channelNumber]->preamble;
}

uint32_t
MmWaveWaveChannelManager::GetManagementPowerLevel (uint32_t channelNumber)
{
  NS_LOG_FUNCTION (this << channelNumber);
  return m_channels[channelNumber]->txPowerLevel;
}

} // namespace ns3
