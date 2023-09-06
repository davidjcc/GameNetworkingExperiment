#include "bs/server_client_manager.h"
#include "bs/server_client.h"

#include <enet/enet.h>

namespace bs {
	server_client_ptr Server_Client_Manager::add_client(_ENetPeer* peer) {
		ASSERT_PANIC(peer != nullptr, "Trying to add client but the peer is NULL");

		client_id id = (client_id)m_clients.size();

		auto inserted = m_clients.insert({ peer, std::make_shared<Server_Client>(peer, id, m_logger) });

		return m_clients[peer];
	}

	void Server_Client_Manager::disconnect_client(const _ENetPeer* peer) {
		for (auto& [id, client] : m_clients) {
			if (id == peer) {
				client->disconnect();
				return;
			}
		}

		UNREACHABLE();
	}

	_ENetPacket* Server_Client_Manager::create_enet_packet(const Packet& packet, bool reliable) {
		const auto bytes = packet.get_bytes();
		ASSERT_PANIC(!bytes.empty(), "Trying to broadcast to all clients but the data is empty");

		enet_uint32 flags = 0;
		if (reliable) {
			flags |= ENET_PACKET_FLAG_RELIABLE;
		}

		auto* enet_packet = enet_packet_create(bytes.data(), bytes.size(), flags);
		ASSERT_PANIC(enet_packet != nullptr, "Error creating packet");
		return enet_packet;
	}

	void Server_Client_Manager::send(server_client_ptr client, _ENetPacket* packet) {
		enet_peer_send(client->get_peer(), 0, packet);
	}

	void Server_Client_Manager::broadcast_to_clients(const Packet& packet, bool reliable) {
		auto* enet_packet = create_enet_packet(packet, reliable);

		for (auto& [_, client] : m_clients) {
			send(client, enet_packet);
		}
	}


	void Server_Client_Manager::broadcast_to_client(server_client_ptr& client, const Packet& packet, bool reliable) {
		auto* enet_packet = create_enet_packet(packet, reliable);
		send(client, enet_packet);
	}

	void Server_Client_Manager::broadcast_to_client(client_id& id, const Packet& packet, bool reliable) {
		auto* enet_packet = create_enet_packet(packet, reliable);
		auto client = get_client(packet.get_peer());
		send(client, enet_packet);
	}
}
