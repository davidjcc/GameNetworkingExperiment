#pragma once

#include <enet/enet.h>
#include <string>
#include <vector>

#include "utils.h"

class Event {
public:
	NO_COPY_NO_MOVE(Event);

	enum Type {
		NONE = 0,
		CONNECT,
		DISCONNECT,
		EVENT_RECEIVED,
	};

	Event(const ENetEvent& event)
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

	~Event();

	ENetPeer* get_peer() const { return m_peer; }
	Type get_type() const { return m_type; }

	std::string get_string() const;
	std::vector<uint8_t> get_bytes() const;

private:
	Type m_type = NONE;
	ENetPeer* m_peer;
	std::vector<uint8_t> m_bytes;
};
