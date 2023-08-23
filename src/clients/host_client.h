#pragma once

#include <enet/enet.h>
#include <spdlog/spdlog.h>

#include "clients/base.h"
#include "utils.h"
#include "event.h"

class Host_Client : public Base_Client {
public:
	NO_COPY_NO_MOVE(Host_Client);

	using poll_callback = void(*)(Host_Client*, Event*);

	Host_Client(logger_t& logger);
	~Host_Client();

	void on_connect(Event& event);
	void on_disconnect(Event& event);

	ENetHost* get_host() const { return m_client; }
	void poll(uint32_t timeout, poll_callback cb);

	bool connect(const char* host, int32_t port);

private:
	ENetHost* m_client = nullptr;
	ENetPeer* m_server = nullptr;
	int32_t m_port;
	const char* m_host;
};

