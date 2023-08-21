#include "server.h"

#include <spdlog/spdlog.h>
#include <enet/enet.h>
#include <nlohmann/json.hpp>

#include <string>
#include <array>
#include <iostream>
#include <list>

inline void panic(const std::string& msg, std::shared_ptr<spdlog::logger>& logger) {
	logger->error(msg);
	exit(EXIT_FAILURE);
}

Game_Server::Game_Server(const char* host, int32_t port, int32_t max_clients, std::shared_ptr<spdlog::logger>& logger)
	: m_host(host), m_port(port), m_max_clients(max_clients), m_logger(logger), m_client_manager(logger)
{
	if (enet_initialize() != 0) {
		panic("An error occurred while initializing ENet.", m_logger);
	}
}

Game_Server::~Game_Server() {
	enet_deinitialize();
}

void Game_Server::start() {
	enet_address_set_host(&m_address, m_host);
	m_address.port = m_port;

	if (m_server = enet_host_create(&m_address, m_max_clients, 0, 0); m_server == NULL) {
		panic("An error occurred while trying to create an ENet server.", m_logger);
	}

	m_logger->info("Server now running on {}:{}", m_host, m_port);
}

void Game_Server::on_client_connect(const ENetEvent& event) {
	Game_Client client();
}

void Game_Server::on_client_disconnect(const ENetEvent& event) {
}

void Game_Server::poll(game_server_callback_t cb) {
	ENetEvent event{};
	while (enet_host_service(m_server, &event, 1000) > 0) {
		switch (event.type) {

		case ENET_EVENT_TYPE_CONNECT: {
			on_client_connect(event);
		} break;

		case ENET_EVENT_TYPE_RECEIVE: {
		} break;

		case ENET_EVENT_TYPE_DISCONNECT: {
			on_client_disconnect(event);
		} break;

		default: break;
		}
	}
}
