#pragma once
#include "clients/server_client.h"
#include "utils.h"
#include "packet.h"

#include <spdlog/logger.h>
#include <enet/enet.h>

#include <unordered_map>


class Server_Client_Manager {
public:
	NO_COPY_NO_MOVE(Server_Client_Manager);

	Server_Client_Manager(logger_t& logger)
		: m_logger(logger) {}

	server_client_ptr add_client(ENetPeer* peer);
	void disconnect_client(const ENetPeer* peer);

	server_client_ptr get_client(ENetPeer* peer) {
		if (m_clients.find(peer) != m_clients.end()) {
			return m_clients[peer];
		}

		return nullptr;
	}

	void broadcast_to_clients(const Packet& packet, bool reliable);
	void broadcast_to_client(server_client_ptr& client, const Packet& packet, bool reliable);
	void broadcast_to_client(client_id& id, const Packet& packet, bool reliable);
private:
	void send(server_client_ptr client, ENetPacket* packet);
	ENetPacket* create_enet_packet(const Packet& packet, bool reliable);

	std::unordered_map<ENetPeer*, server_client_ptr> m_clients;
	logger_t m_logger;
};

