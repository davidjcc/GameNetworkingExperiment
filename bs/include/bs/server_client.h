#pragma once

#include "base.h"
#include "utils.h"

#include <spdlog/spdlog.h>

#include <memory>

#include "enet_fwd.h"

namespace bs {
	class Server_Client : public Base_Client {
	public:
		NO_COPY_NO_MOVE(Server_Client);

		Server_Client(_ENetPeer* peer, const client_id slot, logger_t logger)
			: m_peer(peer), Base_Client(slot, logger)
		{ }

		_ENetPeer* get_peer() const { return m_peer; }

	private:
		_ENetPeer* m_peer = nullptr;
	};

	using server_client_ptr = std::shared_ptr<Server_Client>;
}
