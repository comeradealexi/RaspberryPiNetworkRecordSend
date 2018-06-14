#include "NetworkDestination.h"



NetworkDestination::NetworkDestination(const char* szIP) : m_strIP(szIP)
{	
	int rv;
	int numbytes;



	memset(&m_hints, 0, sizeof m_hints);
	m_hints.ai_family = AF_UNSPEC;
	m_hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(m_strIP.c_str(), RaspberryPi::VideoSurveillance::k_Port, &m_hints, &m_servinfo)) != 0)
	{

		Log::Get() << "getaddrinfo failed with error: " << gai_strerror(rv);
		return;
	}

	auto& p = m_p;

	for (p = m_servinfo; p != NULL; p = p->ai_next) {
		if ((m_socket = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		Log::Get() << "talker: failed to create socket";
		return;
	}

	Log::Get() << "Created Network for IP: " << szIP << " SocketFD: " << m_socket;
}


NetworkDestination::~NetworkDestination()
{
	if (m_socket != -1)
	{
		freeaddrinfo(m_servinfo);
		close(m_socket);
	}

}

int NetworkDestination::SendData(const std::vector<char>& dataSend)
{
	int numbytes;
	if ((numbytes = sendto(m_socket, dataSend.data(), dataSend.size(), 0, m_p->ai_addr, m_p->ai_addrlen)) == -1)
	{
		Log::Get() << "Failed to send packet. NumBytes=" << numbytes;
	}
	else
	{
		Log::Get() << "Sent " << numbytes << " bytes to " << m_strIP.c_str();
	}
	return numbytes;
}
