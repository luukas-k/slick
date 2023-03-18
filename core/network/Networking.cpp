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
			intptr_t offset = 0;

			while (!st.stop_requested()) {
				sockaddr from{};
				i32 len = sizeof(from);
				i32 rlen = recvfrom(mSocket, data + offset, 1024 - offset, 0, &from, &len);

				if (rlen > 0) {
					u32 tid = *(u32*)data;
					offset += rlen;

					if (mTypeSizes.contains(tid)) {
						u32 size = mTypeSizes[tid];
						if (offset >= size) {
							if (mMessageHandlers.contains(tid)) {
								mMessageHandlers[tid](data, size);
								memmove(data, data + size, offset - size);
								offset -= size;
							}
						}
					}

					// Utility::Log("[Con]: Received", std::string(data, rlen));
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
		mSocket(INVALID_SOCKET),
		mMaxConnectionId(1)
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

			auto find_client = [&](sockaddr_in s) -> u32 {
				for (auto& c : mClients) {
					if (s.sin_addr.S_un.S_addr == c.ip && s.sin_port == s.sin_port) {
						return c.conn_id;
					}
				}
				mClients.push_back(Address{
					.ip = s.sin_addr.S_un.S_addr,
					.port = s.sin_port,
					.conn_id = mMaxConnectionId
				});
				mMaxConnectionId += 1;
				
				return mClients[mClients.size() - 1].conn_id;
			};

			while (!t.stop_requested()) {
				sockaddr_in from{};
				i32 fromLen = sizeof(sockaddr);
				i32 res = recvfrom(mSocket, data + offset, 1024 - offset, 0, (sockaddr*)&from, &fromLen);

				if (res > 0) {
					Utility::Log("family: ", from.sin_family, "addr: ", from.sin_addr.S_un.S_addr, "port: ", from.sin_port);
					offset += res;
					u32 msg = *(u32*)data;
					if (mTypeSizes.contains(msg)) {
						u32 size = mTypeSizes[msg];
						if (offset >= size) {
							if (mMessageHandlers.contains(msg)) {
								mMessageHandlers[msg](data, size, find_client(from));
								memmove(data, data + size, offset - size);
								offset -= size;
							}
						}
					}
					
					// sendto(mSocket, data, res, 0, (sockaddr*)&from, sizeof(sockaddr));
					// Utility::Log("[Serv]: Received", std::string(data, res));
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

	void Server::send_data(const char* data, size_t len, u32 conn_id) {
		if(conn_id != 0){
			for (auto& c : mClients) {
				if (c.conn_id == conn_id) {
					sockaddr_in saddr{
						.sin_family = AF_INET,
						.sin_port = c.port,
						.sin_addr = { .S_un = { .S_addr = c.ip } },
					};
					sendto(mSocket, data, len, 0, (const sockaddr*)&saddr, sizeof(saddr));
				}
			}
		}
		else {
			for (auto& c : mClients) {
				sockaddr_in saddr{
					.sin_family = AF_INET,
					.sin_port = c.port,
					.sin_addr = { .S_un = { .S_addr = c.ip } },
				};
				sendto(mSocket, data, len, 0, (const sockaddr*)&saddr, sizeof(saddr));
			}
		}
	}

}
