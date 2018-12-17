/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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

#ifndef SPECTRUM_WIFI_HELPER_H
#define SPECTRUM_WIFI_HELPER_H

#include "wifi-helper.h"

namespace ns3 {

class SpectrumChannel;

/**
 * \brief Make it easy to create and manage PHY objects for the spectrum model.
 *
 * The Pcap and ascii traces generated by the EnableAscii and EnablePcap methods defined
 * in this class correspond to PHY-level traces and come to us via WifiPhyHelper
 *
 */
class SpectrumWifiPhyHelper : public WifiPhyHelper
{
public:
  /**
   * Create a phy helper without any parameter set. The user must set
   * them all to be able to call Install later.
   */
  SpectrumWifiPhyHelper ();

  /**
   * Create a phy helper in a default working state.
   * \returns a default SpectrumWifPhyHelper
   */
  static SpectrumWifiPhyHelper Default (void);

  /**
   * \param channel the channel to associate to this helper
   *
   * Every PHY created by a call to Install is associated to this channel.
   */
  void SetChannel (Ptr<SpectrumChannel> channel);
  /**
   * \param channelName The name of the channel to associate to this helper
   *
   * Every PHY created by a call to Install is associated to this channel.
   */
  void SetChannel (std::string channelName);

  /**
   * \param node the node on which we wish to create a wifi PHY
   * \param device the device within which this PHY will be created
   * \returns a newly-created PHY object.
   *
   * This method implements the pure virtual method defined in \ref ns3::WifiPhyHelper.
   */
  virtual Ptr<WifiPhy> Create (Ptr<Node> node, Ptr<NetDevice> device) const;

private:

  Ptr<SpectrumChannel> m_channel; ///< the channel
};

} //namespace ns3

#endif /* SPECTRUM_WIFI_HELPER_H */
