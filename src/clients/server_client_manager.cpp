#include "clients/server_client_manager.h"
#include "clients/server_client.h"

server_client_ptr Server_Client_Manager::add_client(ENetPeer& peer) {
	// TODO(DC): Generate better ids.
	client_id id = m_clients.size();

	m_clients.insert({ id, std::make_shared<Server_Client>(&peer, m_clients.size(), m_logger) });

	return m_clients[id];
}

ENetPacket* Server_Client_Manager::create_enet_packet(const Packet& packet, bool reliable) {
	const auto bytes = packet.get_bytes();
	ASSERT_PANIC(!bytes.empty(), "Trying to broadcast to all clients but the data is empty");

	enet_uint32 flags = 0;
	if (reliable) {
		flags |= ENET_PACKET_FLAG_RELIABLE;
	}

	ENetPacket* enet_packet = enet_packet_create(bytes.data(), bytes.size(), flags);
	ASSERT_PANIC(enet_packet != nullptr, "Error creating packet");
	return enet_packet;
}

void Server_Client_Manager::send(server_client_ptr client, ENetPacket* packet) {
	enet_peer_send(client->get_peer(), 0, packet);
}

void Server_Client_Manager::broadcast_to_clients(const Packet& packet, bool reliable) {
	ENetPacket* enet_packet = create_enet_packet(packet, reliable);

	for (auto& [_, client] : m_clients) {
		send(client, enet_packet);
	}
}


void Server_Client_Manager::broadcast_to_client(server_client_ptr& client, const Packet& packet, bool reliable) {
	ENetPacket* enet_packet = create_enet_packet(packet, reliable);
	send(client, enet_packet);
}

void Server_Client_Manager::broadcast_to_client(client_id& id, const Packet& packet, bool reliable) {
	ENetPacket* enet_packet = create_enet_packet(packet, reliable);
	auto client = get_client(id);
	send(client, enet_packet);
}
