#pragma once
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
#include <string>

class NetworkDestination
{
public:
	NetworkDestination(const char* szIP);
	~NetworkDestination();

	int SendData(const std::vector<char>& dataSend);

	struct addrinfo m_hints;
	struct addrinfo* m_servinfo;
	struct addrinfo* m_p;
	int m_socket = -1;
	std::string m_strIP;
};

