#include "server.h"

#include <spdlog/spdlog.h>
#include <enet/enet.h>
#include <nlohmann/json.hpp>

#include <string>
#include <array>
#include <iostream>
#include <list>

#include "utils.h"
#include "event.h"

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

void Game_Server::on_client_connect(Event& event) {
	auto client = m_client_manager.add_client(*event.get_peer());
	ASSERT_PANIC(client != nullptr, "Error trying to add new client");
	client->connect();
}

void Game_Server::on_client_disconnect(Event& event) {
	auto client = m_client_manager.get_client_from_event(event);
	ASSERT_PANIC(client != nullptr, "{}: Client not found", __FUNCTION__);
}

void Game_Server::poll(poll_callback cb) {
	ENetEvent enet_event{};
	while (enet_host_service(m_server, &enet_event, 1000) > 0) {
		Event event(enet_event);

		switch (event.get_type()) {
		case Event::CONNECT: {
			on_client_connect(event);
		} break;

		case Event::DISCONNECT: {
			on_client_disconnect(event);
		} break;

		case Event::EVENT_RECEIVED: {
			auto client = m_client_manager.get_client_from_event(event);
			cb(this, client.get(), &event);
		} break;
		}
	}
}

void Game_Server::broadcast_to_all(const Event& event, bool reliable) {
	const auto bytes = event.get_bytes();

	enet_uint32 flags = 0;
	if (reliable) {
		flags |= ENET_PACKET_FLAG_RELIABLE;
	}

	ENetPacket* packet = enet_packet_create(bytes.data(), bytes.size(), flags);
	ASSERT_PANIC(packet != nullptr, "Error creating packet");
}
