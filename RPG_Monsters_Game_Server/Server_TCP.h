//Server_TCP.h
#pragma once

#pragma comment(lib, "ws2_32.lib")			// Winsock library file
#include <WS2tcpip.h>						// Header file for Winsock functions
#include <string>
#include <iostream>
#include <thread>

class Server_TCP;

// Callback to data received
typedef void(*MessageRecievedHandler)(Server_TCP* listener, SOCKET socketId, std::string msg);

class Server_TCP {

private:
	std::string		m_ipAddress;			// IP Address of the server
	int				m_port;					// Listening port # on the server
	sockaddr_in		m_hint;
	bool			m_run_thread_running;	//end thread control or run control
	std::thread		m_run_thread;
	SOCKET			m_listening_sock;

	fd_set			master;					// Create the master file descriptor 

	// Message received event handler, just a function pointer to handle it externally from class
	MessageRecievedHandler	MessageReceived;

	// Create a socket for send/recv
	SOCKET CreateSocket();

public:

	Server_TCP(std::string ipAddress, int port, MessageRecievedHandler handler);

	~Server_TCP();

	// Initialize winsock
	bool Init();

	//send message to 1 client
	bool Send(SOCKET clientSocket, std::string message);

	void SendToAll(std::string message);

	void RunInThread();

	void Run();

};