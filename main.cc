#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"


/* Requisitos:
 * a) ao menos duas redes Ethernet (padrão 802.3)
 * b) quatro redes sem fio (padrão 802.11x)
 * c) um mínimo de 10 clientes em todas as redes
 *
 * Os ambientes das redes (a) e (b) devem ser domínios diferentes de colisão. Na rede 802.3 deve existir um servidor de
 * aplicação que precisa ser acessado por nós/clientes das outras redes. A escolha do serviço a ser implementado é livre.
 */

/*
 * 
 *    Wifi 10.1.2.0
 *                 AP
 *  *    *     *    *    
 *  |    | ... |    |--|
 *  n    n     n    n  |----------------|- n    n     n    n
 *		               |    10.1.1.0    |  |    | ... |    |
 *    Wifi 10.1.3.0    | point-to-point |  =================
 *                  AP |                |    LAN 10.1.6.0
 *  *    *     *    *  |                |    
 *  |    | ... |    |--|                |- n    n     n    n    
 *  n    n     n    n  |	            |  |    | ... |    |
 *                     |                |  =================
 *    Wifi 10.1.4.0    |                |    LAN 10.1.7.0  
 *                  AP |                
 *  *    *     *    *  |                
 *  |    | ... |    |--|                
 *  n    n     n    n  |	
 *                     |
 *    Wifi 10.1.5.0    |              
 *                  AP |                
 *  *    *     *    *  |                
 *  |    | ... |    |--|                
 *  n    n     n    n  |		 	
 * 
 */

int main(int argc,char *argv[]){
	
	uint32_t nNode = 6; //num de nos na simulação
	uint32_t nWiNo = 4; //num de nos sem fio
	uint32_t nEtNo = 2; //num de nos ethernet
	uint32_t nWifi = 10; // numero de client em cada rede sem fio
	uint32_t nEthe = 10; // numero de cliente em cada rede eth
		
	//Nos no dominio de colisão
	NodeContainer p2pNodes;
	p2pNodes.Create (nNode);
	
	
	//Redes sem fio 802.11e
	NodeContainer wifiStaNode[nWiNo];
	NodeContainer wifiApNode[nWifi];
	for(int i = 0; i < nWifi; i++){
		wifiStaNode[i].Create (nWifi);
		wifiApNode[i] = p2pNodes.get(i);
	}

	//Configura e crua um canal wifi 
	YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
	YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
	phy.SetChannel (channel.Create ());
	

	WifiHelper wifi; //the default standard of 802.11a will be selected by this helper since the program doesn't specify another one
	wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
	WifiMacHelper mac;
	
	//Ponto de acesso, um por nó sem fio
	NetDeviceContainer apDevices[nWiNo];
	//clientes nWifi por nó
	NetDeviceContainer staDevices[nWiNo][nWifi];

	Ssid ssid;

	for(int i = 0; i < nWiNo; i++){
		ssid = Ssid("Wifi-"+std::to_string(i));
		
  		mac.SetType ("ns3::StaWifiMac",
  		             "Ssid", SsidValue (ssid),
  		             "ActiveProbing", BooleanValue (false));

  		staDevices = wifi.Install (phy, mac, wifiStaNodes[i]);

  		mac.SetType ("ns3::ApWifiMac",
  		             "Ssid", SsidValue (ssid));
  		apDevices = wifi.Install (phy, mac, wifiApNode[i]);
	
	}
	


}









