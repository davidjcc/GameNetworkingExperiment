#pragma once

#include "server.h"
#include "client.h"
#include "event.h"
#include "utils.h"

class ENet {
public:
	NO_COPY_NO_MOVE(ENet);

	ENet(std::shared_ptr<spdlog::logger>& logger);
	~ENet();

	void init();

	Game_Server* create_server(const char* host, int port, int max_clients);
	void destroy_server(Game_Server* server);

	Host_Client* create_host_client();
	void destroy_client(Game_Client* client);

private:
	enum State {
		STATE_UNINITIALISED = 0,
		STATE_INITIALISED
	} m_state = STATE_UNINITIALISED;

	std::shared_ptr<spdlog::logger> m_logger;
};

