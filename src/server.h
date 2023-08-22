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

	Game_Server(const char* host, int32_t port, int32_t max_clients, std::shared_ptr<spdlog::logger>& logger);
	~Game_Server();

	void set_logger(std::shared_ptr<spdlog::logger>& logger) {
		m_logger = logger;
	}

	std::shared_ptr<spdlog::logger>& get_logger() {
		return m_logger;
	}

	void start();
	void poll(poll_callback cb);

	void broadcast_to_all(const Event& event, bool reliable);

private:
	void on_client_connect(Event& event);
	void on_client_disconnect(Event& event);

	ENetHost* m_server = nullptr;
	ENetAddress m_address{};
	std::shared_ptr<spdlog::logger> m_logger;

	const char* m_host;
	int32_t m_port = 0;
	int32_t m_max_clients = 0;

	Game_Client_Manager m_client_manager;
};
