#pragma once
#include "client.h"
#include "utils.h"

#include <spdlog/logger.h>

#include <unordered_map>


class Game_Client_Manager {
public:
	NO_COPY_NO_MOVE(Game_Client_Manager);

	Game_Client_Manager(logger_t& logger)
		: m_logger(logger) {}

	client_ptr add_client(ENetPeer& peer);

	client_ptr get_client(const client_id id) {
		if (m_clients.find(id) != m_clients.end()) {
			return m_clients[id];
		}

		return nullptr;
	}
private:
	std::unordered_map<client_id, client_ptr> m_clients;
	logger_t m_logger;
};

