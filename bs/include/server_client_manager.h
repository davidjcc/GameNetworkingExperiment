#pragma once
#include "server_client.h"
#include "utils.h"
#include "packet.h"

#include <spdlog/logger.h>

#include <unordered_map>

#include "enet_fwd.h"

class Server_Client_Manager {
public:
	NO_COPY_NO_MOVE(Server_Client_Manager);

	Server_Client_Manager(logger_t& logger)
		: m_logger(logger) {}

	server_client_ptr add_client(_ENetPeer* peer);
	void disconnect_client(const _ENetPeer* peer);

	server_client_ptr get_client(_ENetPeer* peer) {
		if (m_clients.find(peer) != m_clients.end()) {
			return m_clients[peer];
		}

		return nullptr;
	}

	bool empty() const { return m_clients.empty(); }
	bool size() const { return m_clients.size(); }

	void broadcast_to_clients(const Packet& packet, bool reliable);
	void broadcast_to_client(server_client_ptr& client, const Packet& packet, bool reliable);
	void broadcast_to_client(client_id& id, const Packet& packet, bool reliable);
private:
	void send(server_client_ptr client, _ENetPacket* packet);
	_ENetPacket* create_enet_packet(const Packet& packet, bool reliable);

	std::unordered_map<_ENetPeer*, server_client_ptr> m_clients;
	logger_t m_logger;
};

