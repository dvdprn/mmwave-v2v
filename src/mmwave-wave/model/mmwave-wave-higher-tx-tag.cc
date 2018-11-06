/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 * Copyright (c) 2013 Dalian University of Technology
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
 *         Junling Bu <linlinjavaer@gmail.com>
 */
#include "mmwave-wave-higher-tx-tag.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MmWaveWaveHigherLayerTxVectorTag");

NS_OBJECT_ENSURE_REGISTERED (MmWaveWaveHigherLayerTxVectorTag);

TypeId
MmWaveWaveHigherLayerTxVectorTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MmWaveWaveHigherLayerTxVectorTag")
    .SetParent<Tag> ()
    .SetGroupName ("MmWave-Wave")
    .AddConstructor<MmWaveWaveHigherLayerTxVectorTag> ()
  ;
  return tid;
}

TypeId
MmWaveWaveHigherLayerTxVectorTag::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

MmWaveWaveHigherLayerTxVectorTag::MmWaveWaveHigherLayerTxVectorTag (void)
  : m_adaptable (false)
{
  NS_LOG_FUNCTION (this);
}

MmWaveWaveHigherLayerTxVectorTag::MmWaveWaveHigherLayerTxVectorTag (WifiTxVector txVector, bool adaptable)
  : m_txVector (txVector),
    m_adaptable (adaptable)
{
  NS_LOG_FUNCTION (this);
}

WifiTxVector
MmWaveWaveHigherLayerTxVectorTag::GetTxVector (void) const
{
  NS_LOG_FUNCTION (this);
  return m_txVector;
}

bool
MmWaveWaveHigherLayerTxVectorTag::IsAdaptable (void) const
{
  NS_LOG_FUNCTION (this);
  return m_adaptable;
}

uint32_t
MmWaveWaveHigherLayerTxVectorTag::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return (sizeof (WifiTxVector) + 1);
}

void
MmWaveWaveHigherLayerTxVectorTag::Serialize (TagBuffer i) const
{
  NS_LOG_FUNCTION (this << &i);
  i.Write ((uint8_t *)&m_txVector, sizeof (WifiTxVector));
  i.WriteU8 (static_cast<uint8_t> (m_adaptable));
}

void
MmWaveWaveHigherLayerTxVectorTag::Deserialize (TagBuffer i)
{
  NS_LOG_FUNCTION (this << &i);
  i.Read ((uint8_t *)&m_txVector, sizeof (WifiTxVector));
  m_adaptable = i.ReadU8 ();
}

void
MmWaveWaveHigherLayerTxVectorTag::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << " TxVector=" << m_txVector << ";  Adapter=" << m_adaptable;
}

} // namespace ns3
