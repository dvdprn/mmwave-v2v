/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 North Carolina State University
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
 * Author: Scott E. Carpenter <scarpen@ncsu.edu>
 *
 */


#include "ns3/mmwave-wave-bsm-stats.h"
#include "ns3/integer.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveWaveBsmStats");

MmWaveWaveBsmStats::MmWaveWaveBsmStats ()
  : m_wavePktSendCount (0),
    m_waveByteSendCount (0),
    m_wavePktReceiveCount (0),
    m_log (0)
{
  m_wavePktExpectedReceiveCounts.resize (10, 0);
  m_wavePktInCoverageReceiveCounts.resize (10, 0);
  m_waveTotalPktExpectedReceiveCounts.resize (10, 0);
  m_waveTotalPktInCoverageReceiveCounts.resize (10, 0);
}

/* static */
TypeId
MmWaveWaveBsmStats::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveWaveBsmStats")
    .SetParent<Object> ()
    .SetGroupName ("Stats")
    .AddConstructor<MmWaveWaveBsmStats> ()
    ;
  return tid;
}
  
void
MmWaveWaveBsmStats::IncTxPktCount ()
{
  m_wavePktSendCount++;
}

int
MmWaveWaveBsmStats::GetTxPktCount ()
{
  return m_wavePktSendCount;
}

void
MmWaveWaveBsmStats::IncExpectedRxPktCount (int index)
{
  m_wavePktExpectedReceiveCounts[index - 1]++;
  m_waveTotalPktExpectedReceiveCounts[index - 1]++;
}

void
MmWaveWaveBsmStats::IncRxPktCount ()
{
  m_wavePktReceiveCount++;
}

void
MmWaveWaveBsmStats::IncRxPktInRangeCount (int index)
{
  m_wavePktInCoverageReceiveCounts[index - 1]++;
  m_waveTotalPktInCoverageReceiveCounts[index - 1]++;
}

int
MmWaveWaveBsmStats::GetRxPktCount ()
{
  return m_wavePktReceiveCount;
}

int
MmWaveWaveBsmStats::GetExpectedRxPktCount (int index)
{
  return m_wavePktExpectedReceiveCounts[index - 1];
}

int
MmWaveWaveBsmStats::GetRxPktInRangeCount (int index)
{
  return m_wavePktInCoverageReceiveCounts[index - 1];
}

void
MmWaveWaveBsmStats::SetTxPktCount (int count)
{
  m_wavePktSendCount = count;
}

void
MmWaveWaveBsmStats::SetRxPktCount (int count)
{
  m_wavePktReceiveCount = count;
}

void
MmWaveWaveBsmStats::IncTxByteCount (int bytes)
{
  m_waveByteSendCount += bytes;
}

int
MmWaveWaveBsmStats::GetTxByteCount ()
{
  return m_waveByteSendCount;
}

double
MmWaveWaveBsmStats::GetBsmPdr (int index)
{
  double pdr = 0.0;

  if (m_wavePktExpectedReceiveCounts[index - 1] > 0)
    {
      pdr = (double) m_wavePktInCoverageReceiveCounts[index - 1] / (double) m_wavePktExpectedReceiveCounts[index - 1];
      // due to node movement, it is
      // possible to receive a packet that is not slightly "within range" that was
      // transmitted at the time when the nodes were slightly "out of range"
      // thus, prevent overflow of PDR > 100%
      if (pdr > 1.0)
        {
          pdr = 1.0;
        }
    }

  return pdr;
}

double
MmWaveWaveBsmStats::GetCumulativeBsmPdr (int index)
{
  double pdr = 0.0;

  if (m_waveTotalPktExpectedReceiveCounts[index - 1] > 0)
    {
      pdr = (double) m_waveTotalPktInCoverageReceiveCounts[index - 1] / (double) m_waveTotalPktExpectedReceiveCounts[index - 1];
      // due to node movement, it is
      // possible to receive a packet that is not slightly "within range" that was
      // transmitted at the time when the nodes were slightly "out of range"
      // thus, prevent overflow of PDR > 100%
      if (pdr > 1.0)
        {
          pdr = 1.0;
        }
    }

  return pdr;
}

void
MmWaveWaveBsmStats::SetLogging (int log)
{
  m_log = log;
}

int
MmWaveWaveBsmStats::GetLogging ()
{
  return m_log;
}

void
MmWaveWaveBsmStats::SetExpectedRxPktCount (int index, int count)
{
  m_wavePktExpectedReceiveCounts[index - 1] = count;
}

void
MmWaveWaveBsmStats::SetRxPktInRangeCount (int index, int count)
{
  m_wavePktInCoverageReceiveCounts[index - 1] = count;
}

void
MmWaveWaveBsmStats::ResetTotalRxPktCounts (int index)
{
  m_waveTotalPktInCoverageReceiveCounts[index - 1] = 0;
  m_waveTotalPktExpectedReceiveCounts[index - 1] = 0;
}

} // namespace ns3
