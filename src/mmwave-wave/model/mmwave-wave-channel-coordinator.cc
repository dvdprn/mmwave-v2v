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
#include "mmwave-wave-channel-coordinator.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveWaveChannelCoordinator");

/****************************************************************
 *       This destructor is needed.
 ****************************************************************/

MmWaveWaveChannelCoordinationListener::~MmWaveWaveChannelCoordinationListener (void)
{
}

/****************************************************************/

NS_OBJECT_ENSURE_REGISTERED (MmWaveWaveChannelCoordinator);

TypeId
MmWaveWaveChannelCoordinator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveWaveChannelCoordinator")
    .SetParent<Object> ()
    .SetGroupName ("MmWave-Wave")
    .AddConstructor<MmWaveWaveChannelCoordinator> ()
    .AddAttribute ("CchInterval", "CCH Interval, default value is 50ms.",
                   TimeValue (GetDefaultCchInterval ()),
                   MakeTimeAccessor (&MmWaveWaveChannelCoordinator::m_cchi),
                   MakeTimeChecker ())
    .AddAttribute ("SchInterval", "SCH Interval, default value is 50ms.",
                   TimeValue (GetDefaultSchInterval ()),
                   MakeTimeAccessor (&MmWaveWaveChannelCoordinator::m_schi),
                   MakeTimeChecker ())
    .AddAttribute ("GuardInterval", "Guard Interval, default value is 4ms.",
                   TimeValue (GetDefaultGuardInterval ()),
                   MakeTimeAccessor (&MmWaveWaveChannelCoordinator::m_gi),
                   MakeTimeChecker ())
  ;
  return tid;
}

MmWaveWaveChannelCoordinator::MmWaveWaveChannelCoordinator ()
  : m_guardCount (0)
{
  NS_LOG_FUNCTION (this);
}

MmWaveWaveChannelCoordinator::~MmWaveWaveChannelCoordinator ()
{
  NS_LOG_FUNCTION (this);
}

void
MmWaveWaveChannelCoordinator::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  StartChannelCoordination ();
}

void
MmWaveWaveChannelCoordinator::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  StopChannelCoordination ();
  UnregisterAllListeners ();
}

Time
MmWaveWaveChannelCoordinator::GetDefaultCchInterval (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  // refer to Annex H of IEEE 1609.4-2010
  const static uint8_t DEFAULT_CCH_INTERVAL = 50;
  return MilliSeconds (DEFAULT_CCH_INTERVAL);
}

Time
MmWaveWaveChannelCoordinator::GetDefaultSchInterval (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  // refer to Annex H of IEEE 1609.4-2010
  const static uint8_t DEFAULT_SCH_INTERVAL = 50;
  return MilliSeconds (DEFAULT_SCH_INTERVAL);
}

Time
MmWaveWaveChannelCoordinator::GetDefaultSyncInterval (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return GetDefaultCchInterval  () + GetDefaultSchInterval  ();
}

Time
MmWaveWaveChannelCoordinator::GetDefaultGuardInterval (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  // refer to Annex H of IEEE 1609.4-2010
  const static uint8_t SyncTolerance = 2;
  const static uint8_t MaxChSwitchTime  = 2;
  const static uint8_t DEFAULT_GUARD_INTERVAL = SyncTolerance + MaxChSwitchTime;
  return MilliSeconds (DEFAULT_GUARD_INTERVAL);
}

void
MmWaveWaveChannelCoordinator::SetCchInterval (Time cchInterval)
{
  NS_LOG_FUNCTION (this << cchInterval);
  m_cchi = cchInterval;
}

Time
MmWaveWaveChannelCoordinator::GetCchInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_cchi;
}

void
MmWaveWaveChannelCoordinator::SetSchInterval (Time schInterval)
{
  NS_LOG_FUNCTION (this << schInterval);
  m_schi = schInterval;
}

Time
MmWaveWaveChannelCoordinator::GetSchInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_schi;
}

Time
MmWaveWaveChannelCoordinator::GetSyncInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return GetCchInterval () + GetSchInterval ();
}

void
MmWaveWaveChannelCoordinator::SetGuardInterval (Time guard)
{
  NS_LOG_FUNCTION (this);
  m_gi =  guard;
}

Time
MmWaveWaveChannelCoordinator::GetGuardInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_gi;
}

Time
MmWaveWaveChannelCoordinator::GetSchSlot (void) const
{
  NS_LOG_FUNCTION (this);
  return m_schi - m_gi;
}

Time
MmWaveWaveChannelCoordinator::GetCchSlot (void) const
{
  NS_LOG_FUNCTION (this);
  return m_cchi - m_gi;
}

bool
MmWaveWaveChannelCoordinator::IsCchInterval (Time duration) const
{
  NS_LOG_FUNCTION (this << duration);
  Time future = GetIntervalTime (duration);
  return (future < m_cchi);
}

bool
MmWaveWaveChannelCoordinator::IsSchInterval (Time duration) const
{
  NS_LOG_FUNCTION (this << duration);
  return !IsCchInterval (duration);
}

bool
MmWaveWaveChannelCoordinator::IsGuardInterval (Time duration) const
{
  NS_LOG_FUNCTION (this << duration);
  Time future = GetIntervalTime (duration);
  // the interval is either in CchInterval or SchInterval
  Time interval = future < m_cchi ? future : future - m_cchi;
  return interval < m_gi;
}

bool
MmWaveWaveChannelCoordinator::IsValidConfig (void) const
{
  NS_LOG_FUNCTION (this);
  if (GetCchInterval ().GetMilliSeconds () == 0 || GetSchInterval ().GetMilliSeconds () == 0
      || GetGuardInterval ().GetMilliSeconds () == 0)
    {
      NS_LOG_WARN ("the channel interval should not be zero");
      return false;
    }
  // 1000 is 1000ms which is one UTC second
  if ((1000 % GetSyncInterval ().GetMilliSeconds ()) != 0)
    {
      NS_LOG_WARN ("every UTC second shall be an integer number of SyncInterval");
      return false;
    }
  if (GetCchInterval () <= GetGuardInterval ())
    {
      NS_LOG_WARN ("CCH Interval should be large than GuardInterval");
      return false;
    }
  if (GetSchInterval () <= GetGuardInterval ())
    {
      NS_LOG_WARN ("SCH Interval should be large than GuardInterval");
      return false;
    }
  // at last, GguardInterval should be larger than real channel switch time of PHY layer.
  // However there is no method such as GetChannelSwitchTime in the WifiPhy to support test here.
  return true;
}

Time
MmWaveWaveChannelCoordinator::NeedTimeToCchInterval (Time duration) const
{
  NS_LOG_FUNCTION (this << duration);
  if (IsCchInterval (duration))
    {
      return MilliSeconds (0);
    }
  return GetSyncInterval () - GetIntervalTime (duration);
}

Time
MmWaveWaveChannelCoordinator::NeedTimeToSchInterval (Time duration) const
{
  NS_LOG_FUNCTION (this << duration);
  if (IsSchInterval (duration))
    {
      return MilliSeconds (0);
    }
  return GetCchInterval () - GetIntervalTime (duration);
}

Time
MmWaveWaveChannelCoordinator::NeedTimeToGuardInterval (Time duration) const
{
  NS_LOG_FUNCTION (this << duration);
  if (IsGuardInterval (duration))
    {
      return MilliSeconds (0);
    }
  if (IsCchInterval (duration))
    {
      // the time to Guard Interval of SCH Interval
      return (GetCchInterval () - GetIntervalTime (duration));
    }
  // the time to Guard Interval of next CCH Interval
  return (GetSyncInterval () - GetIntervalTime (duration));
}

Time
MmWaveWaveChannelCoordinator::GetIntervalTime (Time duration) const
{
  NS_LOG_FUNCTION (this << duration);
  Time future = Now () + duration;
  Time sync = GetSyncInterval ();
  uint32_t n = future.GetMilliSeconds () / sync.GetMilliSeconds ();
  return future - MilliSeconds (n * sync.GetMilliSeconds ());
}

Time
MmWaveWaveChannelCoordinator::GetRemainTime (Time duration) const
{
  NS_LOG_FUNCTION (this << duration);
  return GetSyncInterval () - GetIntervalTime (duration);
}

void
MmWaveWaveChannelCoordinator::RegisterListener (Ptr<MmWaveWaveChannelCoordinationListener> listener)
{
  NS_LOG_FUNCTION (this << listener);
  NS_ASSERT (listener != 0);
  m_listeners.push_back (listener);
}

void
MmWaveWaveChannelCoordinator::UnregisterListener (Ptr<MmWaveWaveChannelCoordinationListener> listener)
{
  NS_LOG_FUNCTION (this << listener);
  NS_ASSERT (listener != 0);
  for (ListenersI i = m_listeners.begin (); i != m_listeners.end (); ++i)
    {
      if ((*i) == listener)
        {
          m_listeners.erase (i);
          return;
        }
    }
}

void
MmWaveWaveChannelCoordinator::UnregisterAllListeners (void)
{
  NS_LOG_FUNCTION (this);
  m_listeners.clear ();
}

void
MmWaveWaveChannelCoordinator::StartChannelCoordination (void)
{
  NS_LOG_FUNCTION (this);
  Time now = Now ();
  if ((now.GetMilliSeconds () % 1000) != 0)
    {
      // see chapter 5.5.2
      NS_FATAL_ERROR ("the coordination event order should start with the beginning of 1 second");
    }
  if (!IsValidConfig ())
    {
      NS_FATAL_ERROR ("the channel intervals configured for channel coordination events are invalid");
    }
  m_guardCount = 0;
  NotifyGuardSlot ();
}

void
MmWaveWaveChannelCoordinator::StopChannelCoordination (void)
{
  if (!m_coordination.IsExpired ())
    {
      m_coordination.Cancel ();
    }
  m_guardCount = 0;
}

void
MmWaveWaveChannelCoordinator::NotifySchSlot (void)
{
  NS_LOG_FUNCTION (this);
  m_coordination = Simulator::Schedule (GetSchSlot (), &MmWaveWaveChannelCoordinator::NotifyGuardSlot, this);
  for (ListenersI i = m_listeners.begin (); i != m_listeners.end (); ++i)
    {
      (*i)->NotifySchSlotStart (GetSchSlot ());
    }
}

void
MmWaveWaveChannelCoordinator::NotifyCchSlot (void)
{
  NS_LOG_FUNCTION (this);
  m_coordination = Simulator::Schedule (GetCchSlot (), &MmWaveWaveChannelCoordinator::NotifyGuardSlot, this);
  for (ListenersI i = m_listeners.begin (); i != m_listeners.end (); ++i)
    {
      (*i)->NotifyCchSlotStart (GetCchSlot ());
    }
}

void
MmWaveWaveChannelCoordinator::NotifyGuardSlot (void)
{
  NS_LOG_FUNCTION (this);
  Time guardSlot = GetGuardInterval ();
  bool inCchi = ((m_guardCount % 2) == 0);
  if (inCchi)
    {
      m_coordination = Simulator::Schedule (guardSlot, &MmWaveWaveChannelCoordinator::NotifyCchSlot, this);
    }
  else
    {
      m_coordination = Simulator::Schedule (guardSlot, &MmWaveWaveChannelCoordinator::NotifySchSlot, this);
    }
  for (ListenersI i = m_listeners.begin (); i != m_listeners.end (); ++i)
    {
      (*i)->NotifyGuardSlotStart (guardSlot, inCchi);
    }
  m_guardCount++;
}

} // namespace ns3
