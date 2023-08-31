#include "server.h"

#include <spdlog/spdlog.h>
#include <enet/enet.h>

#include <string>
#include <array>
#include <iostream>
#include <list>

#include "utils.h"
#include "packet.h"

Host_Server::Host_Server(const char* host, int32_t port, int32_t max_clients, logger_t& logger)
	: m_host(host), m_port(port), m_max_clients(max_clients), m_logger(logger), m_client_manager(logger)
{
}

Host_Server::~Host_Server() {
	enet_host_destroy(m_server);
}

void Host_Server::start() {
	enet_address_set_host(&m_address, m_host);
	m_address.port = m_port;

	if (m_server = enet_host_create(&m_address, m_max_clients, 0, 0, 0); m_server == nullptr) {
		PANIC("An error occurred while trying to create an ENet server.");
	}

	m_logger->info("Server now running on {}:{}", m_host, m_port);
}

void Host_Server::on_client_connect(Packet& packet) {
	auto client = m_client_manager.add_client(packet.get_peer());

	m_logger->info("Adding new client to client manager at slot: {}", client->get_id());
	ASSERT_PANIC(client != nullptr, "Error trying to add new client");
	client->connect();
}

void Host_Server::on_client_disconnect(Packet& packet) {
	m_logger->info("Disconnecting client: {}", (size_t)packet.get_peer());
	m_client_manager.disconnect_client(packet.get_peer());
}

void Host_Server::tick(uint32_t timeout_ms) {
	ENetEvent enet_event{};
	while (enet_host_service(m_server, &enet_event, timeout_ms) > 0) {
		Packet packet(enet_event);

		// If there is a stored client attached to this peer then add that to the packet.
		auto client = m_client_manager.get_client(enet_event.peer);
		if (client) {
			packet.set_client_id(client->get_id());
		}
		m_packets.push_front(packet);

		switch (packet.get_type()) {
		case Packet::CONNECT: {
			on_client_connect(packet);
		} break;

		case Packet::DISCONNECT: {
			on_client_disconnect(packet);
		} break;

		default: break;
		}
	}
}

void Host_Server::broadcast_to_clients(const Packet& packet, bool reliable) {
	m_logger->trace("Broadcasting packet to clients: '{}'", packet.get_string());
	m_client_manager.broadcast_to_clients(packet, reliable);
}
