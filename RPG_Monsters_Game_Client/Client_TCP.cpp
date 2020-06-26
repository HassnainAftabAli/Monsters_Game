//Client_TCP.cpp

#include "Client_TCP.h"

// Initialize winsock
bool Client_TCP::Init()
{
	WSAData data;
	WORD ver = MAKEWORD(2, 2);

	int wsInit = WSAStartup(ver, &data);
	if (wsInit != 0)
	{
		std::cerr << "Can't start Winsock, Err #" << wsInit << std::endl;
		return false;
	}

	return wsInit == 0;
}

// Create a socket for send/recv
SOCKET Client_TCP::CreateSocket()
{
	SOCKET sock_client = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_client != INVALID_SOCKET)
	{
		// Fill in a hint structure
		m_hint.sin_family = AF_INET;
		m_hint.sin_port = htons(m_port);
		inet_pton(AF_INET, m_ipAddress.c_str(), &m_hint.sin_addr);
	}
	else {
		std::cerr << "Can't create socket, Err #" << WSAGetLastError() << std::endl;
		WSACleanup();
	}

	return sock_client;
}

void Client_TCP::ThreadRecv()
{
	m_recv_thread_running = true;
	while (m_recv_thread_running)
	{
		char buf[4096];
		ZeroMemory(buf, 4096);

		int bytesReceived = recv(m_socket, buf, 4096, 0);
		if (bytesReceived > 0)
		{
			if (MessageReceived != NULL)
			{
				MessageReceived(std::string(buf, 0, bytesReceived));
			}
		}
	}
}

Client_TCP::Client_TCP() {
	m_socket = INVALID_SOCKET;
	m_recv_thread_running = false;
}

Client_TCP::~Client_TCP() {
	closesocket(m_socket);
	WSACleanup();
	if (m_recv_thread_running)
	{
		m_recv_thread_running = false;  //stopping loop in ThreadRecv() 
		m_recv_thread.join();			//wait for it to finish properly
	}
}


bool Client_TCP::Connect(std::string ipAddress, int port)
{
	m_ipAddress = ipAddress;
	m_port = port;

	// Initialize winsock 
	if (!Init()) return false;

	//Creating the socket for client to send and recv
	m_socket = CreateSocket();
	if (m_socket == INVALID_SOCKET) return false;

	// Connect to server
	int connResult = connect(m_socket, (sockaddr*)&m_hint, sizeof(m_hint));
	if (connResult == SOCKET_ERROR)
	{
		std::cerr << "Can't connect to server, Err #" << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

bool Client_TCP::Send(std::string message)
{
	if (!message.empty() && m_socket != INVALID_SOCKET)
	{
		return send(m_socket, message.c_str(), message.size() + 1, 0) != SOCKET_ERROR;
	}
	return false;
}

void Client_TCP::ListenRecvInThread(ClientMessageRecievedHandler handler)
{
	MessageReceived = handler;

	//creating the recv thread using lambda's
	this->m_recv_thread = std::thread([&]()
		{
			ThreadRecv();
		});

}

bool Client_TCP::Recv(ClientMessageRecievedHandler handler)
{
	MessageReceived = handler;

	if (m_socket == INVALID_SOCKET) return false;

	char buf[4096];

	// Wait for response
	ZeroMemory(buf, 4096);
	int bytesReceived = recv(m_socket, buf, 4096, 0);
	if (bytesReceived > 0)
	{
		if (MessageReceived != NULL)
		{
			MessageReceived(std::string(buf, 0, bytesReceived));
		}

		// Echo response to console
		//std::cout << "SERVER> " << std::string(buf, 0, bytesReceived) << std::endl;
	}
	return true;
}