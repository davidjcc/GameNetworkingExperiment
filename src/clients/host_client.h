#pragma once
#include <utility>

#include <enet/enet.h>
#include <spdlog/spdlog.h>

#include "clients/base.h"
#include "utils.h"
#include "packet.h"

class Host_Client : public Base_Client {
public:
	using poll_callback = void(*)(Host_Client*, Packet*);

	Host_Client(logger_t& logger);
	~Host_Client();

	void on_connect(Packet& packet);
	void on_disconnect(Packet& packet);

	ENetHost* get_host() const { return m_client; }
	void tick(uint32_t timeout);

	bool connect(const char* host, int32_t port);

	Ts_Packet_Queue& get_packets() { return m_packets; }

	ENetPeer* get_peer() const { return m_peer; }

	void broadcast_to_server(const Packet& packet, bool reliable = true);

private:
	ENetHost* m_client = nullptr;
	ENetPeer* m_server = nullptr;
	ENetPeer* m_peer = nullptr;
	int32_t m_port;
	const char* m_host;

	Ts_Packet_Queue m_packets;
};

