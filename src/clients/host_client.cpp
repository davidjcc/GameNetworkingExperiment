#include "clients/host_client.h"

Host_Client::Host_Client(logger_t& logger)
	: Base_Client(-1, logger)
{
	m_client = enet_host_create(NULL, 1, 2, 0);

	if (!m_client) {
		PANIC("An error occurred while trying to create an Host client");
	}
}

Host_Client::~Host_Client() {
	enet_host_destroy(m_client);
}

bool Host_Client::connect(const char* host, int32_t port) {
	m_host = host;
	m_port = port;

	ENetAddress address;
	enet_address_set_host(&address, m_host);
	address.port = m_port;
	m_server = enet_host_connect(m_client, &address, 2);
	if (!m_server) {
		PANIC("An error occurred while trying to create an Host peer");
		return false;
	}

	return true;
}

void Host_Client::on_connect(Packet& packet) {
	m_logger->info("Client connected");
}

void Host_Client::on_disconnect(Packet& packet) {
	m_logger->info("Client disconnected");
}

void Host_Client::tick(uint32_t timeout_ms) {
	ENetEvent enet_event{};
	while (enet_host_service(m_client, &enet_event, timeout_ms) > 0) {
		Packet packet(enet_event);
		m_peer = enet_event.peer;

		m_packets.push_front(packet);

		switch (packet.get_type()) {
		case Packet::CONNECT: {
			on_connect(packet);
		} break;

		case Packet::DISCONNECT: {
			on_disconnect(packet);
		} break;

		}
	}
}

void Host_Client::broadcast_to_server(const Packet& packet, bool reliable) {
	enet_uint32 flags = 0;
	if (reliable) {
		flags |= ENET_PACKET_FLAG_RELIABLE;
	}

	auto bytes = packet.get_bytes();
	ASSERT_PANIC(!bytes.empty(), "Trying to broadcast to server but the data is empty");
	ENetPacket* enet_packet = enet_packet_create(bytes.data(), bytes.size(), flags);
	ASSERT_PANIC(enet_packet != nullptr, "Error creating packet");
	enet_peer_send(m_peer, 0, enet_packet);
}

