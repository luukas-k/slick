#pragma once

#include "Core.h"

#include <WinSock2.h>

namespace Slick::Net {

	class Connection {
	public:
		Connection();
		~Connection();

		bool connect(const std::string& addr, u32 port);
		void disconnect();
		
		void send(const std::string& msg);

		inline bool is_connected(){ return mSocket != INVALID_SOCKET; }
	private:
		SOCKET mSocket;
		std::jthread mThread;
	};

	class Server {
	public:
		Server();
		~Server();

		bool listen(u32 port);
		void stop();

		inline bool is_active() const { return mActive; }

	private:
		bool mActive;
		SOCKET mSocket;
		std::jthread mServerThread;
	};

}