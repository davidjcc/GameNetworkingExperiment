#include "event.h"
#include "utils.h"

Event::~Event() {
	enet_packet_destroy(m_event.packet);
}

Event::Type Event::get_type() {
	switch (m_event.type) {
	case ENET_EVENT_TYPE_CONNECT: return CONNECT;
	case ENET_EVENT_TYPE_DISCONNECT: return DISCONNECT;
	case ENET_EVENT_TYPE_RECEIVE: return EVENT_RECEIVED;
	case ENET_EVENT_TYPE_NONE: return NONE;
	default: UNREACHABLE();
	}
}

std::string Event::get_string() const {
	std::string result;
	result.assign(m_event.packet->data, m_event.packet->data + m_event.packet->dataLength);
	return result;
}

std::vector<uint8_t> Event::get_bytes() const {
	std::vector<uint8_t> result;
	result.assign(m_event.packet->data, m_event.packet->data + m_event.packet->dataLength);
	return result;
}