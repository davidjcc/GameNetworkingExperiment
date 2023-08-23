#pragma once
#include <enet/enet.h>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <list>
#include <string>

#include "client.h"
#include "event.h"
#include "utils.h"


class Game_Server {
public:
	NO_COPY_NO_MOVE(Game_Server);

	using poll_callback = void(*)(Game_Server* server, Game_Client* client, Event* event);

	Game_Server(const char* host, int32_t port, int32_t max_clients, logger_t& logger);
	~Game_Server();

	void set_logger(logger_t& logger) {
		m_logger = logger;
	}

	logger_t& get_logger() {
		return m_logger;
	}

	void start();
	void poll(uint32_t timeout_ms, poll_callback cb);

	void broadcast_to_all(const Event& event, bool reliable);

private:
	void on_client_connect(Event& event);
	void on_client_disconnect(Event& event);

	ENetHost* m_server = nullptr;
	ENetAddress m_address{};
	logger_t m_logger;

	const char* m_host;
	int32_t m_port = 0;
	int32_t m_max_clients = 0;

	Game_Client_Manager m_client_manager;
};
