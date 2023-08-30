#include "packet.h"
#include "utils.h"

static Packet::Type get_type_from_enet_type(ENetEventType type) {
	switch (type) {
	case ENET_EVENT_TYPE_NONE: return Packet::NONE;
	case ENET_EVENT_TYPE_CONNECT: return Packet::CONNECT;
	case ENET_EVENT_TYPE_DISCONNECT: return Packet::DISCONNECT;
	case ENET_EVENT_TYPE_RECEIVE: return Packet::EVENT_RECIEVED;
	default: UNREACHABLE(); break;
	}
}

Packet::Packet(ENetPeer* peer, void* data, size_t data_length) : m_peer(peer) {
	set_bytes(data, data_length);
}

Packet::Packet(const ENetEvent& event)
	: m_peer(event.peer) {
	if (event.packet && event.packet->dataLength > 0) {
		m_bytes.assign(event.packet->data, event.packet->data + event.packet->dataLength);
	}
	m_type = get_type_from_enet_type(event.type);
}

Packet::Packet(ENetPeer* peer)
	: m_peer(peer) {
}


Packet::~Packet() {
}

std::string Packet::get_string() const {
	std::string result = "";

	if (!m_bytes.empty()) {
		result.assign(reinterpret_cast<const char*>(m_bytes.data()), m_bytes.size());
	}

	return result;
}

std::vector<uint8_t> Packet::get_bytes() const {
	return m_bytes;
}

void Packet::send(bool reliable) {
	ASSERT_PANIC(m_peer, "Peer is null");
	ASSERT_PANIC(!m_bytes.empty(), "Bytes are empty");

	enet_peer_send(m_peer, 0,
		enet_packet_create(m_bytes.data(), m_bytes.size(), (reliable ? ENET_PACKET_FLAG_RELIABLE : 0)));
}
