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
#include "mmwave-wave-default-channel-scheduler.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/wifi-phy.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveWaveDefaultChannelScheduler");

NS_OBJECT_ENSURE_REGISTERED (MmWaveWaveDefaultChannelScheduler);

/**
 * \ingroup wave
 * \brief MmWaveWaveCoordinationListener class
 */
class MmWaveWaveCoordinationListener : public MmWaveWaveChannelCoordinationListener
{
public:
  /**
   * Constructor
   *
   * \param scheduler channel scheduler
   */
  MmWaveWaveCoordinationListener (MmWaveWaveDefaultChannelScheduler * scheduler)
    : m_scheduler (scheduler)
  {
  }
  virtual ~MmWaveWaveCoordinationListener ()
  {
  }
  virtual void NotifyCchSlotStart (Time duration)
  {
    m_scheduler->NotifyCchSlotStart (duration);
  }
  virtual void NotifySchSlotStart (Time duration)
  {
    m_scheduler->NotifySchSlotStart (duration);
  }
  virtual void NotifyGuardSlotStart (Time duration, bool cchi)
  {
    m_scheduler->NotifyGuardSlotStart (duration, cchi);
  }
private:
  MmWaveWaveDefaultChannelScheduler * m_scheduler; ///< the scheduler
};

TypeId
MmWaveWaveDefaultChannelScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveWaveDefaultChannelScheduler")
    .SetParent<MmWaveWaveChannelScheduler> ()
    .SetGroupName ("MmWave-Wave")
    .AddConstructor<MmWaveWaveDefaultChannelScheduler> ()
  ;
  return tid;
}

MmWaveWaveDefaultChannelScheduler::MmWaveWaveDefaultChannelScheduler ()
  : m_channelNumber (0),
    m_extend (EXTENDED_CONTINUOUS),
    m_channelAccess (NoAccess),
    m_waitChannelNumber (0),
    m_waitExtend (0),
    m_coordinationListener (0)
{
  NS_LOG_FUNCTION (this);
}
MmWaveWaveDefaultChannelScheduler::~MmWaveWaveDefaultChannelScheduler ()
{
  NS_LOG_FUNCTION (this);
}

void
MmWaveWaveDefaultChannelScheduler::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  MmWaveWaveChannelScheduler::DoInitialize ();
}

void
MmWaveWaveDefaultChannelScheduler::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_coordinator = 0;
  if (m_coordinationListener != 0)
    {
      m_coordinationListener = 0;
    }
  if (!m_waitEvent.IsExpired ())
    {
      m_waitEvent.Cancel ();
    }
  if (!m_extendEvent.IsExpired ())
    {
      m_waitEvent.Cancel ();
    }
  MmWaveWaveChannelScheduler::DoDispose ();
}

void
MmWaveWaveDefaultChannelScheduler::SetMmWaveWaveNetDevice (Ptr<MmWaveWaveNetDevice> device)
{
  NS_LOG_FUNCTION (this << device);
  MmWaveWaveChannelScheduler::SetMmWaveWaveNetDevice (device);
  std::vector<Ptr<WifiPhy> > phys = device->GetPhys ();
  if (phys.size () > 1)
    {
      NS_LOG_WARN ("The class is only in the context of single-PHY device, while there are more than one PHY devices");
    }
  // since default channel scheduler is in the context of single-PHY, we only use one phy object.
  m_phy = device->GetPhy (0);
  m_coordinator = device->GetMmWaveWaveChannelCoordinator ();
  m_coordinationListener = Create<MmWaveWaveCoordinationListener> (this);
  m_coordinator->RegisterListener (m_coordinationListener);
}

enum ChannelAccess
MmWaveWaveDefaultChannelScheduler::GetAssignedAccessType (uint32_t channelNumber) const
{
  NS_LOG_FUNCTION (this << channelNumber);
  if (m_channelAccess == AlternatingAccess && channelNumber == CCH)
    {
      return AlternatingAccess;
    }
  return (m_channelNumber == channelNumber) ? m_channelAccess : NoAccess;
}


bool
MmWaveWaveDefaultChannelScheduler::AssignAlternatingAccess (uint32_t channelNumber, bool immediate)
{
  NS_LOG_FUNCTION (this << channelNumber << immediate);
  NS_ASSERT (m_channelAccess != NoAccess && m_channelNumber != 0);
  uint32_t sch = channelNumber;

  if (m_channelAccess == ContinuousAccess || m_channelAccess == ExtendedAccess)
    {
      return false;
    }

  if (m_channelAccess == AlternatingAccess)
    {
      if (m_channelNumber != sch)
        {
          return false;
        }
      else
        {
          return true;
        }
    }

  // if we need immediately switch to AlternatingAccess,
  // we switch to specific SCH.
  if ((immediate && m_coordinator->IsSchInterval ()))
    {
      NS_ASSERT (m_channelNumber == CCH);
      SwitchToNextChannel (CCH, sch);
    }

  m_channelNumber = sch;
  m_channelAccess = AlternatingAccess;
  return true;
}

bool
MmWaveWaveDefaultChannelScheduler::AssignContinuousAccess (uint32_t channelNumber, bool immediate)
{
  NS_LOG_FUNCTION (this << channelNumber << immediate);
  NS_ASSERT (m_channelAccess != NoAccess && m_channelNumber != 0);
  uint32_t sch = channelNumber;
  if (m_channelAccess == AlternatingAccess || m_channelAccess == ExtendedAccess)
    {
      return false;
    }

  if (m_channelAccess == ContinuousAccess)
    {
      if (m_channelNumber != sch)
        {
          return false;
        }
      else
        {
          return true;
        }
    }

  // if there is already an wait event for previous non-immediate request
  if (!m_waitEvent.IsExpired ())
    {
      if (m_waitChannelNumber != sch)
        {
          // then the coming new request will be rejected because of FCFS
          return false;
        }
      else
        {
          if (!immediate)
            {
              return true;
            }
          // then cancel this wait event and assign access for request immediately
          m_waitEvent.Cancel ();
        }
    }

  if (immediate || m_coordinator->IsSchInterval ())
    {
      SwitchToNextChannel (m_channelNumber, sch);
      m_channelNumber = sch;
      m_channelAccess = ContinuousAccess;
    }
  else
    {
      Time wait = m_coordinator->NeedTimeToSchInterval ();
      m_waitEvent = Simulator::Schedule (wait, &MmWaveWaveDefaultChannelScheduler::AssignContinuousAccess, this, sch, false);
      m_waitChannelNumber = sch;
    }

  return true;
}

bool
MmWaveWaveDefaultChannelScheduler::AssignExtendedAccess (uint32_t channelNumber, uint32_t extends, bool immediate)
{
  NS_LOG_FUNCTION (this << channelNumber << extends << immediate);
  NS_ASSERT (m_channelAccess != NoAccess && m_channelNumber != 0);
  uint32_t sch = channelNumber;
  if (m_channelAccess == AlternatingAccess || m_channelAccess == ContinuousAccess)
    {
      return false;
    }

  if (m_channelAccess == ExtendedAccess)
    {
      if (m_channelNumber != sch)
        {
          return false;
        }
      else
        {
          // if current remain extends cannot fulfill the requirement for extends
          Time remainTime = Simulator::GetDelayLeft (m_extendEvent);
          uint32_t remainExtends = remainTime / m_coordinator->GetSyncInterval ();
          if (remainExtends > extends)
            {
              return true;
            }
          else
            {
              return false;
            }
        }
    }

  // if there is already an wait event for previous non-immediate request
  if (!m_waitEvent.IsExpired ())
    {
      NS_ASSERT (m_extendEvent.IsExpired ());
      if (m_waitChannelNumber != sch)
        {
          // then the coming new request will be rejected because of FCFS
          return false;
        }
      else
        {
          if (m_waitExtend < extends)
            {
              return false;
            }

          if (immediate)
            {
              // then cancel previous wait event and
              // go to below code to assign access for request immediately
              m_waitEvent.Cancel ();
            }
          else
            {
              return true;
            }
        }
    }

  if (immediate || m_coordinator->IsSchInterval ())
    {
      SwitchToNextChannel (m_channelNumber, sch);
      m_channelNumber = sch;
      m_channelAccess = ExtendedAccess;
      m_extend = extends;

      Time sync = m_coordinator->GetSyncInterval ();
      // the wait time to proper interval will not be calculated as extended time.
      Time extendedDuration = m_coordinator->NeedTimeToCchInterval () + MilliSeconds (extends * sync.GetMilliSeconds ());
      // after end_duration time, MmWaveWaveDefaultChannelScheduler will release channel access automatically
      m_extendEvent = Simulator::Schedule (extendedDuration, &MmWaveWaveDefaultChannelScheduler::ReleaseAccess, this, sch);
    }
  else
    {
      Time wait = m_coordinator->NeedTimeToSchInterval ();
      m_waitEvent = Simulator::Schedule (wait, &MmWaveWaveDefaultChannelScheduler::AssignExtendedAccess, this, sch, extends, false);
      m_waitChannelNumber = sch;
      m_waitExtend = extends;
    }
  return true;
}

bool
MmWaveWaveDefaultChannelScheduler::AssignDefaultCchAccess (void)
{
  NS_LOG_FUNCTION (this);
  if (m_channelAccess == DefaultCchAccess)
    {
      return true;
    }
  if (m_channelNumber != 0)
    {
      // This class does not support preemptive scheduling
      NS_LOG_DEBUG ("channel access is already assigned for other SCHs, thus cannot assign default CCH access.");
      return false;
    }
  // CCH MAC is to attach single-PHY device and wake up for transmission.
  Ptr<MmWaveWaveOcbWifiMac> cchMacEntity = m_device->GetMac (CCH);
  if (Now ().GetMilliSeconds() != 0)
    {
	  m_phy->SetChannelNumber (CCH);
	  Time switchTime = m_phy->GetChannelSwitchDelay ();
	  cchMacEntity->MakeVirtualBusy (switchTime);
    }
  cchMacEntity->SetWifiPhy (m_phy);
  cchMacEntity->Resume ();

  m_channelAccess = DefaultCchAccess;
  m_channelNumber = CCH;
  m_extend = EXTENDED_CONTINUOUS;
  return true;
}

void
MmWaveWaveDefaultChannelScheduler::SwitchToNextChannel (uint32_t curChannelNumber, uint32_t nextChannelNumber)
{
  NS_LOG_FUNCTION (this << curChannelNumber << curChannelNumber);
  if (m_phy->GetChannelNumber () == nextChannelNumber)
    {
      return;
    }
  Ptr<MmWaveWaveOcbWifiMac> curMacEntity = m_device->GetMac (curChannelNumber);
  Ptr<MmWaveWaveOcbWifiMac> nextMacEntity = m_device->GetMac (nextChannelNumber);
  // Perfect channel switch operation among multiple MAC entities in the context of single PHY device.
  // first make current MAC entity in sleep mode.
  curMacEntity->Suspend ();
  // second unattached current MAC entity from single PHY device
  curMacEntity->ResetWifiPhy ();
  // third switch PHY device from current channel to next channel;
  m_phy->SetChannelNumber (nextChannelNumber);
  // Here channel switch time is required to notify next MAC entity
  // that channel access cannot be enabled in channel switch time.
  Time switchTime = m_phy->GetChannelSwitchDelay ();
  nextMacEntity->MakeVirtualBusy (switchTime);
  // four attach next MAC entity to single PHY device
  nextMacEntity->SetWifiPhy (m_phy);
  // finally resume next MAC entity from sleep mode
  nextMacEntity->Resume ();
}

bool
MmWaveWaveDefaultChannelScheduler::ReleaseAccess (uint32_t channelNumber)
{
  NS_LOG_FUNCTION (this << channelNumber);
  NS_ASSERT (m_channelNumber != 0);
  if (m_channelNumber != channelNumber)
    {
      return false;
    }
  // cancel  current SCH MAC activity and assigned default CCH access.
  SwitchToNextChannel (m_channelNumber, CCH);
  m_channelAccess = DefaultCchAccess;
  m_channelNumber = CCH;
  m_extend = EXTENDED_CONTINUOUS;
  if (!m_waitEvent.IsExpired ())
    {
      m_waitEvent.Cancel ();
    }
  if (!m_extendEvent.IsExpired ())
    {
      m_extendEvent.Cancel ();
    }
  m_waitChannelNumber = 0;
  m_waitExtend = 0;
  return true;
}

void
MmWaveWaveDefaultChannelScheduler::NotifyCchSlotStart (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
}

void
MmWaveWaveDefaultChannelScheduler::NotifySchSlotStart (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
}

void
MmWaveWaveDefaultChannelScheduler::NotifyGuardSlotStart (Time duration, bool cchi)
{
  NS_LOG_FUNCTION (this << duration << cchi);
  // only alternating access requires channel coordination events
  if (m_channelAccess != AlternatingAccess)
    {
      return;
    }

  if (cchi)
    {
      SwitchToNextChannel (m_channelNumber, CCH);
      Ptr<MmWaveWaveOcbWifiMac> mac = m_device->GetMac (CCH);
      // see chapter 6.2.5 Sync tolerance
      // a medium busy shall be declared during the guard interval.
      mac->MakeVirtualBusy (duration);
    }
  else
    {
      Ptr<MmWaveWaveOcbWifiMac> mac = m_device->GetMac (m_channelNumber);
      SwitchToNextChannel (CCH, m_channelNumber);
      // see chapter 6.2.5 Sync tolerance
      // a medium busy shall be declared during the guard interval.
      mac->MakeVirtualBusy (duration);
    }
}
} // namespace ns3
