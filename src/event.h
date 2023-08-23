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

	Event(const ENetEvent& event);
	~Event();

	ENetPeer* get_peer() const { return m_peer; }
	Type get_type() const { return m_type; }
	size_t get_client_id() const { return m_client_id; }

	std::string get_string() const;
	std::vector<uint8_t> get_bytes() const;

private:
	size_t m_client_id;
	Type m_type = NONE;
	ENetPeer* m_peer;
	std::vector<uint8_t> m_bytes;
};
