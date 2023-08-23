#include "server.h"

#include <spdlog/spdlog.h>
#include <enet/enet.h>
#include <nlohmann/json.hpp>

#include <string>
#include <array>
#include <iostream>
#include <list>

#include "utils.h"
#include "packet.h"

Game_Server::Game_Server(const char* host, int32_t port, int32_t max_clients, logger_t& logger)
	: m_host(host), m_port(port), m_max_clients(max_clients), m_logger(logger), m_client_manager(logger)
{
}

Game_Server::~Game_Server() {
	enet_host_destroy(m_server);
}

void Game_Server::start() {
	enet_address_set_host(&m_address, m_host);
	m_address.port = m_port;

	if (m_server = enet_host_create(&m_address, m_max_clients, 0, 0); m_server == nullptr) {
		PANIC("An error occurred while trying to create an ENet server.");
	}

	m_logger->info("Server now running on {}:{}", m_host, m_port);
}

void Game_Server::on_client_connect(Packet& packet) {
	auto client = m_client_manager.add_client(*packet.get_peer());

	m_logger->info("Adding new client to client manager at slot: {}", client->get_slot());
	ASSERT_PANIC(client != nullptr, "Error trying to add new client");
	client->connect();
}

void Game_Server::on_client_disconnect(Packet& packet) {
	//auto client = m_client_manager.get_client_from_event(event);
	//ASSERT_PANIC(client != nullptr, "{}: Client not found", __FUNCTION__);
	TODO;
}

void Game_Server::tick(uint32_t timeout_ms) {
	ENetEvent enet_event{};
	while (enet_host_service(m_server, &enet_event, timeout_ms) > 0) {
		Packet packet(enet_event);
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

void Game_Server::broadcast_to_all(const Packet& packet, bool reliable) {
	const auto bytes = packet.get_bytes();

	enet_uint32 flags = 0;
	if (reliable) {
		flags |= ENET_PACKET_FLAG_RELIABLE;
	}

	ENetPacket* enet_packet = enet_packet_create(bytes.data(), bytes.size(), flags);
	ASSERT_PANIC(enet_packet != nullptr, "Error creating packet");
}
