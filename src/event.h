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
		: m_event(event) {}

	~Event();

	ENetPeer* get_peer() const { return m_event.peer; }
	Type get_type();

	std::string get_string() const;
	std::vector<uint8_t> get_bytes() const;

	template <typename T>
	T* to() {
		ASSERT_PANIC(
			sizeof(m_event.packet->dataLength) == sizeof(T),
			"Trying to parse Type {} of size {} but data length is of size {}",
			typeid(T), sizeof(T), m_event.packet->dataLength);
		return *reinterpret_cast<T*>(m_event.packet->data);
	}

private:
	ENetEvent m_event;
};
