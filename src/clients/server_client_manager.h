#pragma once
#include "clients/server_client.h"
#include "utils.h"

#include <spdlog/logger.h>
#include <enet/enet.h>

#include <unordered_map>


class Server_Client_Manager {
public:
	NO_COPY_NO_MOVE(Server_Client_Manager);

	Server_Client_Manager(logger_t& logger)
		: m_logger(logger) {}

	server_client_ptr add_client(ENetPeer& peer);

	server_client_ptr get_client(const client_id id) {
		if (m_clients.find(id) != m_clients.end()) {
			return m_clients[id];
		}

		return nullptr;
	}
private:
	std::unordered_map<client_id, server_client_ptr> m_clients;
	logger_t m_logger;
};

