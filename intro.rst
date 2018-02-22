
User Association Problem and the need for RRM Module
****************************************************
For a mobile client to associate with the best Access Point and for an AP to be deployed in an appropriate channel for best performace, certain mertics need to measured. 

We have identified the following metrics:

1. Signal to Noise Ratio (SNR)  value of beacons received from neighbouring APs
2. Channel number and the number of Aps in each channel
3. Load on AP ie number of clients  associated to an AP

RRI ( Radio Resource Information)  Module:
*******************************************
We have developed a  RRI Module as an independent class that makes the appropriate  measurements for any device. No changes are made  to the base NS3 classes . The RRI module is implemented at the MAC high layer , same  as that of **ApWifiMac** and the  **StaWifiMac layer**. The measurement details are stored in separate datastructures at this layer. These measurements can be accessed from the application layer and decisions can be made based on these measurements.

Implemention Of RRI Module
**************************
As measurement MAC  class is implemented at the same layer as  **ApWifiMac** and **StaWifiMac**  class, it is also  derived from the  **ns3 ::RegularWifiMac** class. The measurement MAC is deployed in each node along with the **ApWifiMac** class or the **StaWifiMac** class. This class receives all the data packets and the beacons . The class has a function to enable the device to switch to different channels to make the measurements.

Some of the functions Defined in the RRI Module
************************************************

* **Receive()**: This processes both the data packet and beacon , retrieves the appropriate measurement details like RSSI and stores them in appropriate datastructures.

* **Scan()**: This function enables the switching of measurement Mac to different channels to make appropriate measurement.

* **GetSnrDetails()** :  This funcion returns the map data structure that holds the SNR measurement details

Script based implementation
*****************************

In this implementation, the network scenario for deploying the wifinodes and the logic to make the appropriate decision is implemeted in a single Ns3 script file. The client node is installed with  both the wifi Mac object and the scan mac object  that makes the measurements. Appropriate functions are implemented in the script to make the decisions and change the association of the client.

For example , a function AssociateWithBestSNR() is implemented in the script.  This accessses the  underlying datasructure objects created by the measurement mac object . The  AP with the best snr for this client is found by accessing the datastructure. From the script, the AP with the best SNR for the client is found and the client is associated to that AP.

Reference :
***********

**Title:** A Simulation Framework for Radio Resource Management in WiFi Networks

**Url:** http://www.ee.iitm.ac.in/~rvenkat/ants-2017-rri.pdf


