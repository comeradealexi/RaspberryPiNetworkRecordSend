#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <RaspberryPiShared.h>
#include <vector>
#include <thread>
#include <wiringPi.h>
#include <iostream>
#include <memory>
#include "NetworkDestination.h"
#include <algorithm>

const char k_networkFileName[] = "networks.txt";
const auto k_sleepTime = std::chrono::milliseconds(500);

// LED Pin - wiringPi pin 0 is BCM_GPIO 17.
// we have to use BCM numbering when initializing with wiringPiSetupSys
// when choosing a different pin number please use the BCM numbering, also
// update the Property Pages - Build Events - Remote Post-Build Event command 
// which uses gpio export for setup for wiringPiSetupSys
#define	LED	17

//Essentially all this program does is send UDP packets to specific devices on the network when the door status changes.

int main()
{
	Log::Get() << "Starting NetworkRecordSend";

	std::vector<std::unique_ptr<NetworkDestination>> netDevices;

	Log::Get() << "Reading in network devices from " << k_networkFileName;

	std::ifstream inputFile(k_networkFileName);
	if (inputFile.good())
	{
		std::string strLine;
		while (std::getline(inputFile, strLine))
		{
			strLine.erase(std::remove(strLine.begin(), strLine.end(), '\n'), strLine.end());
			strLine.erase(std::remove(strLine.begin(), strLine.end(), '\r'), strLine.end());

			Log::Get() << "Read Line: " << strLine;
			netDevices.push_back(std::make_unique<NetworkDestination>(strLine.c_str()));
		}
	}

	if (netDevices.size() == 0)
	{
		Log::Get() << "Terminating Program as no networks loaded.";
		return EXIT_FAILURE;
	}

	if (wiringPiSetupSys() != 0)
		Log::Get() << "wiringPiSetupSys returned error code" << "\n";

	pinMode(LED, INPUT);

	static int iPrev = digitalRead(LED);

	while(1)
	{
		int numbytes;

		int iRead = digitalRead(LED);
		if (iRead != 0 && iRead != 1)
		{
			Log::Get() << "digitalRead returned weird value of: " << iRead;
		}

		if (iPrev != iRead)
		{
			RaspberryPi::VideoSurveillance::DoorStatus ds = iRead == 0 ? RaspberryPi::VideoSurveillance::DoorStatus::Open : RaspberryPi::VideoSurveillance::DoorStatus::Closed;
			uint32_t uiData = (uint32_t)ds;
			std::vector<char> sendBuffer;
			sendBuffer.resize(sizeof(uiData) + RaspberryPi::VideoSurveillance::k_PacketHeaderSize);
			RaspberryPi::VideoSurveillance::CreatePacket(sendBuffer.data(), uiData);

			Log::Get() << "Door status changed to: " << RaspberryPi::VideoSurveillance::DoorStatusToString(ds);
			Log::Get() << "Sending packet.";

			for (auto& n : netDevices)
				numbytes = n->SendData(sendBuffer);
			
			iPrev = iRead;
		}

		std::this_thread::sleep_for(k_sleepTime);
	}

	return 0;
}