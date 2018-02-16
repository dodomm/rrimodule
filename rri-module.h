/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
* Copyright (c) 2017 Indian Institute Of Technology Madras
*               2017 Electrical Engineering Department
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
* RRI Module
*
* Authors:		: Dr. Venkatesh Ramaiyan
*
* Implemented By:  Krishna Bharadwaj, <pvnskbs@gmail.com>
*                      S.Kalpalatha <skalpaiit@gmail.com>
*
*
*
*/


#ifndef RRI_MODULE_H
#define RRI_MODULE_H

#include "regular-wifi-mac.h"
#include "supported-rates.h"
#include "capability-information.h"


namespace ns3  {

/**
*  Structure to record the details when a beacon is received
*/
typedef struct
{
  double snrValue;
  Ssid   apSsid;
  int    chnlNum;
  Time   crntTime;
} SnrMsrmtDetails;

/**
*  Structure to record the details during data communication 
*/
typedef struct
{
  Mac48Address apMac;
  int              chnlNum;
  std::string 	   dataPath;
  Time             crntTime;

} LoadMsrmtDetails;

class RriModule : public RegularWifiMac
{
public:
  static TypeId GetTypeId (void);

  RriModule ();
  virtual ~RriModule ();


/**
* This function is declared are pure virtual function in the base class i.e RegularWifiMac.
* Hence this class has to implement that function
*/
  virtual void Enqueue (Ptr<const Packet> packet, Mac48Address to);

// Added for Scanning
  void Scan (); //!< Scanning- Function to  scan channels
  void SetChanneltoScan (); //!< - Function to  set different channels to scan
  void ScheduleEvent (bool enable);

//Return the map data structure that holds the SNR measurement details
  std::map <Mac48Address, std::list<SnrMsrmtDetails> > GetSnrDetails ();

//Retun the map data structure that holds the load details
  std::map <Mac48Address, LoadMsrmtDetails> GetLoadDetails ();


private:
  void Receive (Ptr<Packet> packet, const WifiMacHeader *hdr);


  EventId  m_scanEvent; //!< Channel scanning

  Time m_startScan; //!< Attribute for  Triggering the scan function at specific time
  Time m_scanDuration; //!< Attribute for duration for scanning each channel
  std::string m_channelToScan; //!< Attribute to store the list of channels to be scanned
  int m_choice; //!< To choose the  channel to scan
  int  *m_chnl; //!< To get the channel number from the array
  int  m_chnlList[10] = { 36,40,44,36,36,36,36,36,36,36}; //!Default Channel numbers
  int  m_noChnls; //!No of Channels to scan


//For recording the AP and the client address
  Mac48Address m_ap_address, m_client_address;
  int m_channel, m_load;

// When device receives beacon
  std::map <Mac48Address, std::list<SnrMsrmtDetails> > mapApSnrSsidChnl; //!<Map to Store 10 values of (SnrValue, ApSsid, Channel No, Ct time)  for each Ap

//When Device receives data.
  std::map <Mac48Address, LoadMsrmtDetails > mapclientApChannel; //!< Store Client Mac Address, AP Mac Address , Channel No, path

};

} //namespace ns3

#endif /* RRI_MODULE_H */
