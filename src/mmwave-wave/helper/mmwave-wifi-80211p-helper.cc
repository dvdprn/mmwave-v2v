/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 * Copyright (c) 2009 MIRKO BANCHI
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
 * Author: Mirko Banchi <mk.banchi@gmail.com>
 * Author: Junling Bu <linlinjavaer@gmail.com>
 */
#include "ns3/string.h"
#include "ns3/log.h"
#include <typeinfo>
#include "mmwave-wave-mac-helper.h"
#include "mmwave-wifi-80211p-helper.h"
#include "ns3/unused.h"

namespace ns3 {

MmWaveWifi80211pHelper::MmWaveWifi80211pHelper ()
{
}

MmWaveWifi80211pHelper::~MmWaveWifi80211pHelper ()
{
}

MmWaveWifi80211pHelper
MmWaveWifi80211pHelper::Default (void)
{
  MmWaveWifi80211pHelper helper;
  helper.SetStandard (WIFI_PHY_STANDARD_80211_10MHZ);
  helper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode", StringValue ("OfdmRate6MbpsBW10MHz"),
                                  "ControlMode",StringValue ("OfdmRate6MbpsBW10MHz"),
                                  "NonUnicastMode", StringValue ("OfdmRate6MbpsBW10MHz"));
  return helper;
}

void
MmWaveWifi80211pHelper::SetStandard (enum WifiPhyStandard standard)
{
  if ((standard == WIFI_PHY_STANDARD_80211a) || (standard == WIFI_PHY_STANDARD_80211_10MHZ))
    {
      WifiHelper::SetStandard (standard);
    }
  else
    {
      NS_FATAL_ERROR ("802.11p only use 802.11 standard with 10MHz or 20MHz");
    }
}


void
MmWaveWifi80211pHelper::EnableLogComponents (void)
{
  WifiHelper::EnableLogComponents ();

  LogComponentEnable ("MmWaveWaveOcbWifiMac", LOG_LEVEL_ALL);
  LogComponentEnable ("MmWaveWaveVendorSpecificAction", LOG_LEVEL_ALL);
}

NetDeviceContainer
MmWaveWifi80211pHelper::Install (const WifiPhyHelper &phyHelper, const WifiMacHelper &macHelper, NodeContainer c) const
{
  QosMmWaveWaveMacHelper const * qosMac = dynamic_cast <QosMmWaveWaveMacHelper const *> (&macHelper);
  if (qosMac == 0)
    {
      NqosMmWaveWaveMacHelper const * nqosMac = dynamic_cast <NqosMmWaveWaveMacHelper const *> (&macHelper);
      if (nqosMac == 0)
        {
          NS_FATAL_ERROR ("the macHelper should be either QosMmWaveWaveMacHelper or NqosMmWaveWaveMacHelper"
                          ", or should be the subclass of QosMmWaveWaveMacHelper or NqosMmWaveWaveMacHelper");
        }
      NS_UNUSED (nqosMac);
    }

  NS_UNUSED (qosMac);
  return WifiHelper::Install (phyHelper, macHelper, c);
}

} // namespace ns3
