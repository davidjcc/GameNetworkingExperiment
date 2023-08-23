#include "event.h"
#include "utils.h"

Event::Event(const ENetEvent& event)
		: m_peer(event.peer) {
		if (event.packet && event.packet->dataLength > 0) {
			m_bytes.assign(event.packet->data, event.packet->data + event.packet->dataLength);
		}

		switch (event.type) {
		case ENET_EVENT_TYPE_NONE: m_type = NONE; break;
		case ENET_EVENT_TYPE_CONNECT: m_type = CONNECT; break;
		case ENET_EVENT_TYPE_DISCONNECT: m_type = DISCONNECT; break;
		case ENET_EVENT_TYPE_RECEIVE: m_type = EVENT_RECEIVED; break;
		default: UNREACHABLE(); break;
		}
	}


Event::~Event() {
}

std::string Event::get_string() const {
	std::string result = "";

	if (!m_bytes.empty()) {
		result.assign(reinterpret_cast<const char*>(m_bytes.data()), m_bytes.size());
	}

	return result;
}

std::vector<uint8_t> Event::get_bytes() const {
	return m_bytes;
}
