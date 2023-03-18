#include "Networking.h"
#include "Networking.h"
#include "Networking.h"


#include "utility/Logger.h"

namespace Slick::Net {

	Connection::Connection() 
		:
		mSocket(INVALID_SOCKET)
	{
		WSAData wsaData{};
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			Utility::Log("WSAStartup() failed");
			return;
		}
	}

	Connection::~Connection() {}

	bool Connection::connect(const std::string& addr, u32 port) {
		mSocket = socket(AF_INET, SOCK_DGRAM, 0);
		if (mSocket == INVALID_SOCKET) {
			Utility::Log("Connection::connect() Failed to create socket.");
			return false;
		}

		sockaddr_in saddr{};
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons(port);
		saddr.sin_addr.S_un.S_addr = inet_addr(addr.c_str());

		DWORD timeout = 50;
		i32 res = setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(DWORD));

		res = ::connect(mSocket, (sockaddr*)&saddr, sizeof(saddr));
		if (res != 0) {
			closesocket(mSocket);
			mSocket = INVALID_SOCKET;
			return false;
		}

		mThread = std::jthread([this](std::stop_token st) {
			char data[1024]{};
			while (!st.stop_requested()) {
				sockaddr from{};
				i32 len = sizeof(from);
				i32 rlen = recvfrom(mSocket, data, 1024, 0, &from, &len);

				if (rlen > 0) {
					Utility::Log("[Con]: Received", std::string(data, rlen));
				}
			}
		});

		return true;
	}

	void Connection::disconnect() {
		mThread.request_stop();
		mThread.join();
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
	}

	void Connection::send_data(const char* msg, size_t len) {
		sendto(mSocket, msg, len, 0, nullptr, 0);
	}

	Server::Server() 
		:
		mActive(false),
		mSocket(INVALID_SOCKET)
	{}

	Server::~Server() {
		mActive = false;
	}

	bool Server::listen(u32 port) {
		mSocket = socket(AF_INET, SOCK_DGRAM, 0);
		if (mSocket == INVALID_SOCKET) {
			Utility::Log("Server::listen() Failed to start server.");
			return false;
		}
		
		sockaddr_in saddr{};
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons(port);
		saddr.sin_addr.S_un.S_addr = INADDR_ANY;

		DWORD timeout = 50;
		i32 res = setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(DWORD));

		if (res != 0) {
			Utility::Log("Server::listen() Failed to set socket options.");
			closesocket(mSocket);
			mSocket = INVALID_SOCKET;
			return false;
		}

		res = bind(mSocket, (sockaddr*)&saddr, sizeof(saddr));
		if (res != 0) {
			Utility::Log("Server::listen() Failed to bind socket.");
			closesocket(mSocket);
			mSocket = INVALID_SOCKET;
			return false;
		}

		mServerThread = std::jthread([this](std::stop_token t) {
			char data[1024]{};
			intptr_t offset = 0;

			while (!t.stop_requested()) {
				sockaddr from{};
				i32 fromLen = sizeof(sockaddr);
				i32 res = recvfrom(mSocket, data + offset, 1024 - offset, 0, &from, &fromLen);

				if (res > 0) {
					offset += res;
					u32 msg = *(u32*)data;
					if (mTypeSizes.contains(msg)) {
						u32 size = mTypeSizes[msg];
						if (offset >= size) {
							if (mMessageHandlers.contains(msg)) {
								mMessageHandlers[msg](data, size);
								memmove(data, data + size, offset - size);
								offset -= size;
							}
						}
					}
					
					// sendto(mSocket, data, res, 0, (sockaddr*)&from, sizeof(sockaddr));
					Utility::Log("[Serv]: Received", std::string(data, res));
				}
			}
		});

		mActive = true;
		return true;
	}

	void Server::stop() {
		mServerThread.request_stop();
		mServerThread.join();
		closesocket(mSocket);
		mActive = false;
	}

}
