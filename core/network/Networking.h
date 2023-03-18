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

		template<typename T>
		void register_type(u32 tid) {
			struct Message {
				u32 tid;
				T v;
			};
			mTypeIds[type_id<T>()] = tid;
			mTypeSizes[type_id<T>()] = sizeof(Message);
		}

		// On message of type T
		template<typename T, typename FN>
		void on(FN&& cb) {
			struct Message {
				u32 tid;
				T v;
			};

			u32 tid = mTypeIds.at(type_id<T>());
			mMessageHandlers[tid] = [cb](const char* data, size_t len) {
				Message* m = (Message*)data;
				cb(m->v);
			};
		}
		// Send message of type T
		template<typename T>
		void send(const T& v) {
			struct Message {
				u32 tid;
				T v;
			} msg{
				.tid = mTypeIds.at(type_id<T>()),
				.v = v
			};
			send_data((const char*)&msg, sizeof(Message));
		}

		inline bool is_connected(){ return mSocket != INVALID_SOCKET; }
	private:
		void send_data(const char* msg, size_t len);
		
	private:
		SOCKET mSocket;
		std::jthread mThread;
		std::unordered_map<u32, u32> mTypeIds;
		std::unordered_map<u32, u32> mTypeSizes;
		std::unordered_map<u32, std::function<void(const char*, size_t)>> mMessageHandlers;
	};

	class Server {
	public:
		Server();
		~Server();

		bool listen(u32 port);
		void stop();

		template<typename T>
		void register_type(u32 tid) {
			struct Message {
				u32 tid;
				T v;
			};
			mTypeIds[type_id<T>()] = tid;
			mTypeSizes[type_id<T>()] = sizeof(Message);
		}

		template<typename FN>
		void on_connect(FN&& fn) {

		}

		template<typename FN>
		void on_disconnect(FN&& fn) {

		}

		// On message of type T
		template<typename T, typename FN>
		void on(FN&& cb) {
			u32 tid = mTypeIds.at(type_id<T>());
			struct Message {
				u32 tid;
				T v;
			};
			mMessageHandlers[tid] = [cb](const char* data, size_t len, u32 conn_id) {
				Message* msg = (Message*)data;
				cb(msg->v, conn_id);
			};
		}
		// Send message of type T
		template<typename T>
		void send(const T& v, u32 conn_id = 0) {
			struct Message {
				u32 tid;
				T v;
			} msg{
				.tid = mTypeIds.at(type_id<T>()),
				.v = v
			};
			send_data((const char*)&msg, sizeof(Message), conn_id);
		}

		inline bool is_active() const { return mActive; }

	private:
		void send_data(const char* data, size_t len, u32 conn_id);

	private:
		bool mActive;
		SOCKET mSocket;
		std::jthread mServerThread;
		std::unordered_map<u32, u32> mTypeIds;
		std::unordered_map<u32, u32> mTypeSizes;
		std::unordered_map<u32, std::function<void(const char*, size_t, u32)>> mMessageHandlers;
		struct Address {
			u32 ip;
			u16 port;

			u32 conn_id;
		};
		std::vector<Address> mClients;
		u32 mMaxConnectionId;
	};

}