#pragma once

#include <enet/enet.h>

#include <string>
#include <vector>
#include <deque>
#include <utility>

#include "utils.h"

class Packet {
public:
	enum Type {
		NONE = 0,
		CONNECT,
		DISCONNECT,
		EVENT_RECIEVED,
	};

	Packet(const ENetEvent& event);
	~Packet();

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

struct Ts_Packet_Queue {
public:
	Ts_Packet_Queue() = default;
	Ts_Packet_Queue(const Ts_Packet_Queue&) = delete;
	virtual ~Ts_Packet_Queue() {
		clear();
	}

	Packet& front() {
		std::scoped_lock lock(m_mutex);
		return m_packets.front();
	}

	Packet& back() {
		std::scoped_lock lock(m_mutex);
		return m_packets.back();
	}

	Packet pop_front() {
		std::scoped_lock lock(m_mutex);

		auto result = std::move(m_packets.front());
		m_packets.pop_front();
		return result;
	}

	void push_back(Packet& packet) {
		std::scoped_lock lock(m_mutex);
		m_packets.push_back(packet);
	}
	void push_front(Packet& packet) {
		std::scoped_lock lock(m_mutex);
		m_packets.push_front(packet);
	}

	void clear() {
		std::scoped_lock lock(m_mutex);
		m_packets.clear();
	}

	size_t size() {
		std::scoped_lock lock(m_mutex);
		return m_packets.size();
	}

	bool empty() {
		std::scoped_lock lock(m_mutex);
		return m_packets.empty();
	}

private:
	std::mutex m_mutex;
	std::deque<Packet> m_packets;
};

