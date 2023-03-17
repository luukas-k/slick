#include "Networking.h"

#include <WinSock2.h>

#include "utility/Logger.h"

namespace Slick::Net {

	Connection::Connection() {
		WSAData wsaData{};
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			Utility::Log("WSAStartup() failed");
			return;
		}


	}

	Connection::~Connection() {}

	void Connection::connect(const std::string& addr, u32 port) {

	}

	Server::Server() 
		:
		mActive(false)
	{}

	Server::~Server() {
		mActive = false;
	}

	bool Server::listen(u32 port) {
		mActive = true;
		return true;
	}

	void Server::stop() {
		mActive = false;
	}

}
