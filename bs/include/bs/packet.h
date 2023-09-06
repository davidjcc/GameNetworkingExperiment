#pragma once

#include <string>
#include <vector>
#include <deque>
#include <utility>

#include "utils.h"

#include "enet_fwd.h"

namespace bs {
	class Packet {
	public:
		enum Type {
			NONE = 0,
			CONNECT,
			DISCONNECT,
			EVENT_RECIEVED,
		};

		Packet(const _ENetEvent* event);
		Packet(_ENetPeer* peer);
		Packet(_ENetPeer* peer, void* data, size_t data_length);
		Packet() = default;
		~Packet();

		_ENetPeer* get_peer() const { return m_peer; }
		void set_peer(_ENetPeer* peer) { m_peer = peer; }

		void set_client_id(int32_t id) { m_client_id = id; }

		Type get_type() const { return m_type; }
		int32_t get_client_id() const { return m_client_id; }

		std::string get_string() const;
		std::vector<uint8_t> get_bytes() const;

		void set_bytes(void* data, size_t length) {
			m_bytes.clear();
			m_bytes.assign(reinterpret_cast<uint8_t*>(data), reinterpret_cast<uint8_t*>(data) + length);
		}

		void set_bytes(const std::vector<uint8_t>& data) { m_bytes = data; }
		void set_string(const std::string& str) { m_bytes.assign(str.begin(), str.end()); }
		void set_type(Type type) { m_type = type; }

		void send(bool reliable);

	private:
		int32_t m_client_id = -1;
		Type m_type = NONE;
		_ENetPeer* m_peer = nullptr;
		std::vector<uint8_t> m_bytes;
	};

	struct Ts_Packet_Queue {
	public:
		Ts_Packet_Queue() = default;
		NO_COPY_NO_MOVE(Ts_Packet_Queue);
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

		Packet pop_back() {
			std::scoped_lock lock(m_mutex);

			auto result = std::move(m_packets.front());
			m_packets.pop_back();
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
}
