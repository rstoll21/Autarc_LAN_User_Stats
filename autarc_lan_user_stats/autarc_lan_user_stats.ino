#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
// See https://github.com/BlakeFoster/Arduino-Ping/
#include "ICMPPing.h"
#include "IPHelper.h"
#include "memcheck.h"
// init_mem();  //hier???
// Serial.println(get_mem_unused());

// WireShark Filter: eth.addr[0:3]==90:A2:DA

// Simon
//static char AVRID[6]           = "Simon";
//static uint8_t mac_shield[6]   = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F };
//byte ip_shield[4]              = { 10, 0, 1, 13 };
//byte gateway[4]                = { 10, 0, 1, 1 };
//byte subnet[4]                 = { 255, 255, 0, 0 };

//// Jonas
//static char AVRID[6]           = "Jonas";
//static uint8_t mac_shield[6]   = { 0x90, 0xA2, 0xDA, 0x00, 0x46, 0x8F };
//byte ip_shield[4]              = { 192, 168, 1, 30 };
//byte gateway[4]                = { 192, 168, 1, 1 };
//byte subnet[4]                 = { 255, 255, 0, 0 };


//// Tim

//EEPROM
// Storenumber |    Variable   | Size
//-----------------------------------
//  0          | configured    | 1
//  1 -  6     | mac_shield    | 6
//  7 - 10     | ip_shield     | 4
// 11 - 14     | gateway       | 4
// 15 - 18     | subnet        | 4
// 19          | useDhcp       | 1
// 20          | pingrequest   | 1
// 21          | useSubnetting | 1
// 22 - 25     | start_ip      | 4
// 26 - 29     | end_ip        | 4
// 30 - 35     | AVRID         | 6



byte mac_shield[6];
byte ip_shield[4];
byte gateway[4];
byte subnet[4];
byte useDhcp;
byte pingrequest;
byte useSubnetting;
byte start_ip[4];
byte end_ip[4];

char AVRID[6];

byte readSubnet[4];
byte readIP[4];

byte currIP[4];
byte currMAC[6];

int configurate;

// Ping library configuration
SOCKET pingSocket              = 0;

// HTTP server in the internet
// EthernetClient client;
// IPAddress server(64, 233, 187, 99);
// IPAddress server(85,10,211,16); // kolchose.org

// Global namespace to use it in setup() and loop()
ICMPPing ping(pingSocket, (uint16_t)random(0, 255));



void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.print("Speicher: ");
  Serial.println(get_mem_unused());
  
//________________________Configuration of the board______________________________

  Serial.print("Press any key to configurate");
  for (char i = 0; i < 3 and Serial.available() <= 0; i++) {
    delay(1000);
    Serial.print(".");
  }
  configurate = Serial.read();
  if (configurate >= 0) {
    Serial.println("Starting configuration");
      Serial.println("MAC Board: ");
      GetMAC(mac_shield);
      write_EEPROM(1, mac_shield , sizeof(mac_shield));
      print_mac(mac_shield);
      Serial.println("Stored");
      Serial.println("\n");
      
      Serial.println("IP Board: ");
      GetIP(ip_shield);
      write_EEPROM(7, ip_shield , sizeof(ip_shield));
      print_ip(ip_shield);
      Serial.println("Stored");
      Serial.println("\n");
      
      Serial.println("IP Gateway: ");
      GetIP(gateway);
      write_EEPROM(11, gateway , sizeof(gateway));
      print_ip(gateway);
      Serial.println("Stored");
      Serial.println("\n");
      
      //TODO: Check if it works fine!
      Serial.println("Subnetmask: ");
      GetIP(subnet);
      write_EEPROM(15, subnet , sizeof(subnet));
      print_ip(subnet);
      Serial.println("Stored");
      Serial.println("\n");
      
      Serial.println("Use DHCP (0 = no): ");
      useDhcp = GetNumber();
      write_EEPROM(19, useDhcp);
      if (useDhcp == 0) {
       Serial.println("Don't use DHCP");
      } else {
        Serial.println("Use DHCP");
      }
      Serial.println("Stored");
      Serial.println("\n");
      
      Serial.println("Number of ping-requests: ");
      pingrequest = GetNumber();
      write_EEPROM(20, pingrequest);
      Serial.print("Number of ping-requests: ");
      Serial.println(pingrequest);
      Serial.println("Stored");
      Serial.println("\n");
      
      Serial.println("Use Subnetting (0 = no): ");
      useSubnetting = GetNumber();
      write_EEPROM(21, useSubnetting);
      if (useSubnetting == 0) {
       Serial.println("Don't use Subnetting");
       Serial.println("Stored");
       Serial.println("\n");
       
          Serial.println("Start IP for Scan: ");
          GetIP(start_ip);
          write_EEPROM(22, start_ip , sizeof(start_ip));
          print_ip(start_ip);
          Serial.println("Stored");
          Serial.println("\n");
       
          Serial.println("End IP for Scan: ");
          GetIP(end_ip);
          write_EEPROM(26, end_ip , sizeof(end_ip));
          print_ip(end_ip);
          Serial.println("Stored");
          Serial.println("\n");
       
      } else {
        Serial.println("Use Subnetting");
        Serial.println("Stored");
        Serial.println("\n");
      }
      
      //TODO: Get free AVR-ID from Server?
      Serial.println("AVR-ID: ");
      GetString(AVRID, sizeof(AVRID));
      write_EEPROM(30, AVRID , sizeof(AVRID));
      Serial.println(AVRID);
      Serial.println("Stored");
      Serial.println("\n");

      
      //Confirm settings and set configured = 1 in EEPROM
      write_EEPROM(0, 1);
      
      
      Serial.println("\n");
      Serial.println("Setup finished");
      Serial.println("\n");
      
    } else {
    Serial.println("no configuration");
  }
  
  
//_____________________Loading the values for the board__________________________
  if (read_EEPROM(0) != 1) {
   //use default values:
      Serial.println("No configuration stored yet. Using default values...");
      mac_shield[0] = 0x90;
      mac_shield[1] = 0xA2;
      mac_shield[2] = 0xDA;
      mac_shield[3] = 0x00;
      mac_shield[4] = 0x46;
      mac_shield[5] = 0x9F;
      ip_shield[0] = 192;
      ip_shield[1] = 168;
      ip_shield[2] = 178;
      ip_shield[3] = 98;
      gateway[0] = 192;
      gateway[1] = 168;
      gateway[2] = 178;
      gateway[3] = 1;
      subnet[0] = 255;
      subnet[1] = 255;
      subnet[2] = 255;
      subnet[3] = 0;
      useDhcp = 1;
      pingrequest = 2;
      useSubnetting = 1;
      start_ip[0] = 192;
      start_ip[1] = 168;
      start_ip[2] = 178;
      start_ip[3] = 2;
      end_ip[0] = 192;
      end_ip[1] = 168;
      end_ip[2] = 178;
      end_ip[3] = 254;
      AVRID[0] = 'T';
      AVRID[1] = 'i';
      AVRID[2] = 'm';
      AVRID[3] = '0';
      AVRID[4] = '0';
      AVRID[5] = '\0';
      
  } else {
   //Read values from EEPROM:
   Serial.println("Using configuration from EEPROM.");
    read_EEPROM(1, mac_shield , sizeof(mac_shield));
    read_EEPROM(7, ip_shield , sizeof(ip_shield));
    read_EEPROM(11, gateway , sizeof(gateway));
    read_EEPROM(15, subnet , sizeof(subnet));
    useDhcp = read_EEPROM(19);
    pingrequest = read_EEPROM(20);
    useSubnetting = read_EEPROM(21);
    read_EEPROM(22, start_ip , sizeof(start_ip));
    read_EEPROM(26, end_ip , sizeof(end_ip));
    read_EEPROM(30, AVRID , sizeof(AVRID));
  }



//________________________Initialising of the board______________________________
  Serial.println("Try to get IP address from network...");
  Serial.print(" MAC address of shield: ");
  print_mac(mac_shield);
  // Setup when no IP is known
  if (useDhcp == 0) {
    Ethernet.begin(mac_shield, ip_shield, gateway, subnet);
  } else {
    if (Ethernet.begin(mac_shield) == 0) {
      Serial.println("DHCP failed, no automatic IP address assigned!");
      Serial.print("Time for waiting for IP address: ");
      Serial.print(millis());
      Serial.println(" ms");
      Serial.println("Trying to set manual IP address.");
      Ethernet.begin(mac_shield, ip_shield, gateway, subnet);
    }
  }

  Serial.println(" Address assigned?");
  Serial.print(" ");
  Serial.println(Ethernet.localIP());
  Serial.print(" ");
  Serial.println(Ethernet.subnetMask());
  Serial.println(" Setup complete\n");
  Serial.print("Speicher: ");
  Serial.println(get_mem_unused());

//TODO: Test with Subnetmask for Subnetting!
//Set start_ip and end_ip if subnetting is choosed
if (useSubnetting != 0) {
    for (byte i = 0; i < 4; i++) {
      readSubnet[i] = Ethernet.subnetMask()[i], DEC;
      readIP[i]     = Ethernet.localIP()[i], DEC;
      start_ip[i]   = readIP[i] & readSubnet[i];
      if (start_ip[i] == 0) {
       start_ip[i] = 1;   //TODO: Set to 2, because of Gateway
      }
      end_ip[i]     = readIP[i] | ~readSubnet[i];
      if (end_ip[i] == 255) {
       end_ip[i] = 254; 
      }
     }
}

  Serial.print("\nStarting loop trough IP range ");
  print_ip(start_ip);
  Serial.print(" - ");
  print_ip(end_ip);
}



//___________________________Scan the network_________________________________
void loop() {
  Serial.print("Speicher (Loop-Start): ");
  Serial.println(get_mem_unused());
  for(int b = 0; b < 4; b++) { 
    currIP[b]  = start_ip[b]; 
  }
  while (1) {
    if (currIP[3] <= end_ip[3]) { // TODO: An mögliche anpassen; mehrere Blöcke
      ICMPEchoReply echoReply = ping(currIP, pingrequest);    
      if (echoReply.status == SUCCESS) {
        // We found a device!
        Serial.print("Speicher (Geraet gefunden): ");
        Serial.println(get_mem_unused());
        for(int mac = 0; mac < 6; mac++) {
          currMAC[mac] = echoReply.MACAddressSocket[mac];
        }
        
        Serial.print("Device found on: ");
        print_ip(currIP);
        Serial.print(" MAC: ");
        print_mac(currMAC);
        
      } else {
        // It's not responding, next one
        for(int mac = 0; mac < 6; mac++) {
          currMAC[mac] = 0;
        }
        Serial.print("No (pingable) device on IP ");
        print_ip(currIP);
      }
      send_info_to_server(currIP, currMAC, AVRID);
      
      currIP[3]++;  
    } else {
      //TODO: next IP-Block?!
      break; // Exit Loop 
    }
  }
  Serial.print("Speicher (Ende ServerSend): ");
  Serial.println(get_mem_unused());
  Serial.println("Restart loop");
}
