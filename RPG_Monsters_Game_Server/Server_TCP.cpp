//Server_TCP.cpp

#include "Server_TCP.h"

SOCKET Server_TCP::CreateSocket()
{
	SOCKET sock_server = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_server != INVALID_SOCKET)
	{
		// Fill in a hint structure
		m_hint.sin_family = AF_INET;
		m_hint.sin_port = htons(m_port);
		inet_pton(AF_INET, m_ipAddress.c_str(), &m_hint.sin_addr);

		// Bind the ip address and port to a socket
		int bindOk = bind(sock_server, (sockaddr*)&m_hint, sizeof(m_hint));
		if (bindOk != SOCKET_ERROR)
		{
			// Tell Winsock the socket is for listening 
			int listenOk = listen(sock_server, SOMAXCONN);
			if (listenOk == SOCKET_ERROR)
			{
				std::cerr << "Can't listen to socket, Err #" << WSAGetLastError() << std::endl;
				return SOCKET_ERROR;
			}
		}
		else {
			std::cerr << "Can't bind to socket, Err #" << WSAGetLastError() << std::endl;
			return SOCKET_ERROR;
		}

	}
	else {
		std::cerr << "Can't create socket, Err #" << WSAGetLastError() << std::endl;
		WSACleanup();
	}

	return sock_server;
}

Server_TCP::Server_TCP(std::string ipAddress, int port, MessageRecievedHandler handler)
	: m_ipAddress(ipAddress), m_port(port), MessageReceived(handler)
{
	FD_ZERO(&master);
	m_run_thread_running = false;
}

Server_TCP::~Server_TCP() {

	//closing all sockets =========================================
	// Message to let users know what's happening.
	std::string msg = "Server is shutting down. Goodbye\r\n";
	while (master.fd_count > 0)
	{
		// Get the socket number
		SOCKET sock = master.fd_array[0];

		// Send the goodbye message
		send(sock, msg.c_str(), (int)(msg.size() + 1), 0);

		// Remove it from the master file list and close the socket
		FD_CLR(sock, &master);
		closesocket(sock);
	}
	//closing all sockets =========================================

	WSACleanup();

	//finishing running thread
	if (m_run_thread_running)
	{
		m_run_thread_running = false;  //stopping loop in ThreadRecv() 
		//std::cout << "Finishing RUN thread..." << std::endl;
		m_run_thread.join();			//wait it to finish properly (naturally)
		//std::cout << "Done." << std::endl;
	}
}

// Initialize winsock
bool Server_TCP::Init()
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

//send message to 1 client
bool Server_TCP::Send(SOCKET clientSocket, std::string message)
{
	if (!message.empty())
	{
		return send(clientSocket, message.c_str(), (int)(message.size() + 1), 0) != SOCKET_ERROR;
	}
	return false;
}

//send message to all clients
void Server_TCP::SendToAll(std::string message)
{
	if (!message.empty())
	{
		for (unsigned int i = 0; i < master.fd_count; i++)
		{

			SOCKET outSock = master.fd_array[i];
			if (outSock != m_listening_sock)
			{
				send(outSock, message.c_str(), (int)(message.size() + 1), 0);
			}
		}
	}
}

void Server_TCP::RunInThread() //unblocked
{
	//creating the run thread using lambda's
	this->m_run_thread = std::thread([&]()
		{
			Run();
		});
}

void Server_TCP::Run()
{
	// Create a listening socket
	m_listening_sock = CreateSocket();
	if (m_listening_sock == INVALID_SOCKET)
	{
		std::cerr << "Error in Run(), could not start server." << std::endl;
		return;
	}

	// Add our first socket that we're interested in interacting with; the listening socket!
	// It's important that this socket is added for our server or else we won't 'hear' incoming
	// connections 
	FD_SET(m_listening_sock, &master);



	// this will be changed by the \quit command (see below, bonus not in video!)
	m_run_thread_running = true;
	std::cout << "Server is running" << std::endl;
	while (m_run_thread_running)
	{

		fd_set copy = master;

		// See who's talking to us
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		// Loop through all the current connections / potential connect
		for (int i = 0; i < socketCount; i++)
		{
			// Makes things easy for us doing this assignment
			SOCKET sock = copy.fd_array[i];

			// Is it an inbound communication?
			if (sock == m_listening_sock)
			{
				// Accept a new connection
				SOCKET client = accept(m_listening_sock, nullptr, nullptr);

				// Add the new connection to the list of connected clients
				FD_SET(client, &master);

				// Send a welcome message to the connected client
				/*std::string welcomeMsg = "Welcome to the communication Server!\r\n";
				send(client, welcomeMsg.c_str(), (int)(welcomeMsg.size() + 1), 0);*/
			}
			else // It's an inbound message
			{
				char buf[4096];
				ZeroMemory(buf, 4096);

				// Receive message
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					// Drop the client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					// Check to see if it's a command. \quit kills the server
					if (buf[0] == '\\')
					{
						// Is the command quit? 
						std::string cmd = std::string(buf, bytesIn);
						if (cmd == "\\quit")
						{
							m_run_thread_running = false;
							break;
						}

						// Unknown command
						continue;
					}

					//handle msg externally from class
					if (MessageReceived != NULL)
					{
						MessageReceived(this, sock, std::string(buf, 0, bytesIn));
					}
				}
			}
		}
	}

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	FD_CLR(m_listening_sock, &master);
	closesocket(m_listening_sock);
	std::cout << "Server stopped" << std::endl;

	// Message to let users know what's happening.
	std::string msg = "Server is shutting down. Goodbye\r\n";

	while (master.fd_count > 0)
	{
		// Get the socket number
		SOCKET sock = master.fd_array[0];

		// Send the goodbye message
		send(sock, msg.c_str(), (int)(msg.size() + 1), 0);

		// Remove it from the master file list and close the socket
		FD_CLR(sock, &master);
		closesocket(sock);
	}
}
