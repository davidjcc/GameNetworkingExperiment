#pragma once

#include "server.h"
#include "host_client.h"
#include "packet.h"
#include "utils.h"

class ENet {
public:
	NO_COPY_NO_MOVE(ENet);

	ENet(logger_t& logger);
	~ENet();

	Host_Server* create_server(const char* host, int port, int max_clients);
	void destroy_server(Host_Server* server);

	Host_Client* create_host_client();
	void destroy_client(Host_Client* client);

private:
	enum State {
		STATE_UNINITIALISED = 0,
		STATE_INITIALISED
	} m_state = STATE_UNINITIALISED;

	logger_t m_logger;
};
