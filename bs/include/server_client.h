#pragma once

#include "base.h"
#include "utils.h"

#include <spdlog/spdlog.h>
#include <enet/enet.h>

#include <memory>

class Server_Client : public Base_Client {
public:
	NO_COPY_NO_MOVE(Server_Client);

	Server_Client(ENetPeer* peer, const client_id slot, logger_t logger)
		: m_peer(peer), Base_Client(slot, logger)
	{ }

	ENetPeer* get_peer() const { return m_peer; }

private:
	ENetPeer* m_peer = nullptr;
};

using server_client_ptr = std::shared_ptr<Server_Client>;
