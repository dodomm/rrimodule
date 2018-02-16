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



//Include header file
#include "rri-module.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "mac-low.h"

#include "ns3/tag.h" //Added Code -UA
#include "ns3/snr-tag.h" //Added Code -UA

#include <stdlib.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RriModule");

NS_OBJECT_ENSURE_REGISTERED (RriModule);

TypeId
RriModule::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RriModule")
    .SetParent<RegularWifiMac> ()
    .SetGroupName ("Wifi")
    .AddConstructor<RriModule> ()

//Specify the start time for the scan logic
    .AddAttribute ("StartTime",
                   "For Triggering the scan function ",
                   TimeValue (Seconds (0.0)),
                   MakeTimeAccessor (&RriModule::m_startScan),
                   MakeTimeChecker ())

//Attribute for setting the duration of scan time
    .AddAttribute ("ScanDuration",
                   "For setting the duration of scan time ",
                   TimeValue (Seconds (0.0)),
                   MakeTimeAccessor (&RriModule::m_scanDuration),
                   MakeTimeChecker ())

//Attribute set to true always. Call the SceduleEvent function
    .AddAttribute ("ScheduleAll",
                   "To schedule all functions",
                   BooleanValue (true),
                   MakeBooleanAccessor (&RriModule::ScheduleEvent),
                   MakeBooleanChecker ())


    //Attribut to get the list of channels to be scanned as string from script
    .AddAttribute ("ChannelToScan",
                   "List of Channels to be Scanned",
                   StringValue ("36,36,36"),
                   MakeStringAccessor (&RriModule::m_channelToScan),
                   MakeStringChecker ())
  ;


  return tid;
}

RriModule::RriModule ()
  : m_choice (0) //To choose the channel to scan

{
  NS_LOG_FUNCTION (this);
}

RriModule::~RriModule ()
{
  NS_LOG_FUNCTION (this);
}

void
RriModule::ScheduleEvent (bool enable)
{
// For scanning the different channels
  m_scanEvent = Simulator::Schedule (m_startScan,&RriModule::Scan,this);

}

void
RriModule::Enqueue (Ptr<const Packet> packet, Mac48Address to)
{
  NS_LOG_FUNCTION (this << packet << to);
}


void
RriModule::Receive (Ptr<Packet> packet, const WifiMacHeader *hdr)
{
  NS_LOG_FUNCTION (this << packet << hdr);
  NS_ASSERT (!hdr->IsCtl ());

  Mac48Address temp_address; // For User Association. (UA)
  temp_address = hdr->GetAddr2 (); // UA  To get the source address from hdr

//For UA
  SnrTag tag;
  double snrValue = 0.0, SNRValueindB = 0.0;
  
     
//To retrieve the snr tag. Header files tag.h and snr-tag.h also added
  if (packet->PeekPacketTag (tag))
    {
      NS_LOG_DEBUG ("Received Packet with SNR = " << tag.Get ());
      snrValue = tag.Get ();
      SNRValueindB = (10 * log10 (snrValue));
    }


  if (hdr->GetAddr3 () == GetAddress ())
    {
      NS_LOG_LOGIC ("packet sent by us.");
      return;
    }

  else if (hdr->IsData ())
    {
      LoadMsrmtDetails ldMsrDet;
      std::string dataPath;

      // For finding the load on AP. 

      // Whenever the client receives or sends data, it indicates that there is a communication between
      // Client and AP. Store the ,channel number, mac address of client and AP in a datastructure

      // This condition is to eliminate olsr packets sent from AP or sent by Client. We do not want those packets
      // as those olsr packets are also treated as data

		//if ( hdr->GetAddr1 () == "00:00:00:00:00:09" )
         // std::cout << " from rri source Address = " << hdr->GetAddr2 () << " destination Address = " << hdr->GetAddr1 () << "  BSSID=  " << hdr->GetAddr3 () << std::endl;
      if ( hdr->GetAddr1 () != "ff:ff:ff:ff:ff:ff" && hdr->GetAddr3 () != "ff:ff:ff:ff:ff:ff")
        {
          //std::cout << "From SCAN MAC: Channel = " << GetWifiPhy ()->GetChannelNumber () << "   from header source Address = " << hdr->GetAddr2 () << "  from header destination Address = " << hdr->GetAddr1 () << "BSSID=  " << hdr->GetAddr3 () << std::endl;
          m_channel = GetWifiPhy ()->GetChannelNumber ();

          //A condition to check who is the SENDER(whether AP or Client)
          if (hdr->IsFromDs ())          //SENDER IS AP
            {
              m_ap_address = hdr->GetAddr2 ();
              m_client_address = hdr->GetAddr1 ();
              dataPath = "DL";
            }
          else                         //SENDER IS CLIENT
            {
              m_ap_address = hdr->GetAddr1 ();
              m_client_address = hdr->GetAddr2 ();
              dataPath = "UL";
            }

          ldMsrDet.apMac    = m_ap_address;
          ldMsrDet.chnlNum  = m_channel;
          ldMsrDet.dataPath = dataPath;
          ldMsrDet.crntTime = Simulator::Now ();

          if (mapclientApChannel.count (m_client_address))
            {
              mapclientApChannel.erase (mapclientApChannel.find (m_client_address));
              mapclientApChannel[m_client_address] = ldMsrDet;

            }
          else
            {
              mapclientApChannel[m_client_address] = ldMsrDet;
            }
        }

      return;
    }

  else if (hdr->IsBeacon ())
    {
      MgtBeaconHeader beacon;
      packet->RemoveHeader (beacon);

      //Added  For Snr based UA

      Ssid apSsid;
      apSsid = beacon.GetSsid ();
      int chnlNo = GetWifiPhy ()->GetChannelNumber ();

      SnrMsrmtDetails msrDet;
      msrDet.snrValue = SNRValueindB;
      msrDet.apSsid = apSsid;
      msrDet.chnlNum = chnlNo;
      msrDet.crntTime = Simulator::Now ();

      // Store the measurement details in the map
      if (mapApSnrSsidChnl.count (temp_address))
        {
          std::list<SnrMsrmtDetails> presentlist;
          presentlist = mapApSnrSsidChnl[temp_address];

          if (presentlist.size () >= 5) // To store only 5 values in the list
            {
              presentlist.pop_front ();
              presentlist.push_back (msrDet);

            }
          else // If count is less than 5, simply push
            {
              presentlist.push_back (msrDet);
            }

          mapApSnrSsidChnl.erase (mapApSnrSsidChnl.find (temp_address));
          mapApSnrSsidChnl[temp_address] = presentlist;
        }
      else // Create new entry if not present in list already
        {
          std::list<SnrMsrmtDetails> presentlist;
          presentlist.push_back (msrDet);
          mapApSnrSsidChnl[temp_address] = presentlist;

        }
      return;
    }

//Invoke the receive handler of our parent class to deal with any
//other frames. Specifically, this will handle Block Ack-related
//Management Action frames.
  RegularWifiMac::Receive (packet, hdr);
}

//Added code -Scanning- for channel scanning by the Device
void
RriModule::Scan ()
{
  Time t = Simulator::Now ();
	//std::cout<<"\n At "<<t<<"	Scanning Channel " << "\n";
	
   // After n number of are channels are scanned, return to channel 1
  if ( m_choice == m_noChnls )
    {
      m_choice = 0;
    }
   
   //std::cout<< "From Scan m_choice = " << m_choice << "\n";
   GetWifiPhy ()->SetChannelNumber (m_chnl[m_choice]);
 
   //std::cout<<GetWifiPhy ()->GetChannelNumber()<<std::endl;

  m_choice++;
  m_scanEvent = Simulator::Schedule (m_scanDuration, &RriModule::Scan,this);
}

//Scanning- to get the list of channels to scan
void RriModule::SetChanneltoScan ()
{
  std::cout << " RRI Module Channel To Scan 1" << "\n";

  std::string chnlScan = m_channelToScan;
  std::string delimiter = ",",token;
  size_t pos = 0;
  int i = 0;

  while ((pos = chnlScan.find (delimiter)) != std::string::npos)
    {
      token = chnlScan.substr (0, pos);
      m_chnlList[i] = atoi (token.c_str ());
      chnlScan.erase (0, pos + delimiter.length ());
      i++;
    }

  m_chnlList[i] = atoi (chnlScan.c_str ());
  m_noChnls = i + 1;
  m_chnl = m_chnlList;

  std::cout<< " RRI Module Channel To Scan  m_noChnls " <<  m_noChnls << "\n";
  //std::cout << "From Setchannel " << m_chnl[0] << "\t"  << m_chnl[1] << "\t"  << m_chnl[2] << "\t" << "\n";
}

std::map <Mac48Address, std::list<SnrMsrmtDetails> > RriModule::GetSnrDetails ()
{
  return mapApSnrSsidChnl;
}

std::map <Mac48Address, LoadMsrmtDetails> RriModule::GetLoadDetails ()
{
  return mapclientApChannel;
}


} //namespace ns3
