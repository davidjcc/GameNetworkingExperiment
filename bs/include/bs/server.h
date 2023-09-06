#pragma once
#include <spdlog/spdlog.h>

#include <cstdint>
#include <list>
#include <string>

#include "server_client_manager.h"
#include "base.h"
#include "packet.h"
#include "utils.h"

#include "enet_fwd.h"

namespace bs {
	class Host_Server {
	public:
		NO_COPY_NO_MOVE(Host_Server);

		Host_Server(const char* host, int32_t port, int32_t max_clients, logger_t& logger);
		~Host_Server();

		void set_logger(logger_t& logger) {
			m_logger = logger;
		}

		logger_t& get_logger() {
			return m_logger;
		}

		void start();
		void tick(uint32_t timeout_ms);

		void broadcast_to_clients(const Packet& packet, bool reliable);

		Ts_Packet_Queue& get_packets() { return m_packets; }

		Server_Client_Manager& get_client_manager() { return m_client_manager; }

	private:
		void on_client_connect(Packet& packet);
		void on_client_disconnect(Packet& packet);

		_ENetHost* m_server = nullptr;
		logger_t m_logger;

		const char* m_host;
		int32_t m_port = 0;
		int32_t m_max_clients = 0;

		Server_Client_Manager m_client_manager;

		Ts_Packet_Queue m_packets;
	};
}
