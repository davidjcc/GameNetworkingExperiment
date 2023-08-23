#pragma once

#include <enet/enet.h>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <list>
#include <string>
#include <memory>
#include <unordered_map>

#include "utils.h"
#include "event.h"

using game_server_callback_t = void(*)(ENetEvent& event);

using client_id = size_t;

class Game_Client {
public:
	NO_COPY_NO_MOVE(Game_Client);

	enum State {
		NONE = 0,
		CONNECTED,
		DISCONNECTED,
	};

	Game_Client(const client_id slot, logger_t& logger);

	const size_t get_slot() const { return m_slot; }
	const State& get_state() const { return m_state; }
	logger_t& get_logger() { return m_logger; }

	void connect() {
		m_state = CONNECTED;
	}

	void disconnect() {
		m_state = DISCONNECTED;
	}

private:
	State m_state = NONE;

	client_id m_slot;
	logger_t m_logger;
};

class Internal_Client : public Game_Client {
public:
	NO_COPY_NO_MOVE(Internal_Client);

	Internal_Client(ENetPeer* peer, const client_id slot, logger_t logger);

	ENetPeer* get_peer() const { return m_peer; }

private:
	ENetPeer* m_peer = nullptr;
};

class Host_Client : public Game_Client {
public:
	NO_COPY_NO_MOVE(Host_Client);

	using poll_callback = void(*)(Host_Client*, Event*);

	Host_Client(logger_t& logger);
	~Host_Client();

	void on_connect(Event& event);
	void on_disconnect(Event& event);

	ENetHost* get_host() const { return m_client; }
	void poll(uint32_t timeout, poll_callback cb);

	bool connect(const char* host, int32_t port);

private:
	ENetHost* m_client = nullptr;
	ENetPeer* m_server = nullptr;
	int32_t m_port;
	const char* m_host;
};

using client_ptr = std::shared_ptr<Internal_Client>;
