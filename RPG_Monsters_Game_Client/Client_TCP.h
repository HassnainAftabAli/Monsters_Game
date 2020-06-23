//Client_TCP.h
#pragma once

#pragma comment(lib, "ws2_32.lib")			// Winsock library file
#include <WS2tcpip.h>						// Header file for Winsock functions
#include <string>
#include <iostream>
#include <thread>

class Client_TCP;

// Callback to data received
typedef void(*ClientMessageRecievedHandler)(std::string msg);

class Client_TCP {

private:
	std::string		m_ipAddress;			// IP Address of the server to connect to
	int				m_port;					// Listening port # on the server
	SOCKET			m_socket;
	sockaddr_in		m_hint;
	bool			m_recv_thread_running;	//thread end control
	std::thread		m_recv_thread;

	// Message received event handler, just a function pointer to handle it externally from class
	ClientMessageRecievedHandler	MessageReceived;

	// Initialize winsock
	bool Init();

	// Create a socket for send/recv
	SOCKET CreateSocket();

	void ThreadRecv();

public:

	Client_TCP();

	~Client_TCP();

	bool Connect(std::string ipAddress, int port);

	bool Send(std::string message);

	void ListenRecvInThread(ClientMessageRecievedHandler handler);

	bool Recv(ClientMessageRecievedHandler handler);
};
