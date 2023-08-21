#pragma once
#include "client.h"

#include <enet/enet.h>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <list>
#include <string>

using game_server_callback_t = void(*)(ENetEvent& event);

class Game_Server {
public:
	Game_Server(const char* host, int32_t port, int32_t max_clients, std::shared_ptr<spdlog::logger>& logger);
	~Game_Server();

	void set_logger(std::shared_ptr<spdlog::logger>& logger) {
		m_logger = logger;
	}

	void start();

	void poll(game_server_callback_t cb);

private:
	void on_client_connect(const ENetEvent& event);
	void on_client_disconnect(const ENetEvent& event);

	ENetHost* m_server = nullptr;
	ENetAddress m_address{};
	std::shared_ptr<spdlog::logger> m_logger;

	const char* m_host;
	int32_t m_port = 0;
	int32_t m_max_clients = 0;

	Game_Client_Manager m_client_manager;
};
