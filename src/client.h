#pragma once

#include <enet/enet.h>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <list>
#include <string>
#include <memory>

using game_server_callback_t = void(*)(ENetEvent& event);

enum Game_Client_State {
	GAME_CLIENT_STATE_NONE = 0,
	GAME_CLIENT_STATE_CONNECTED,
	GAME_CLIENT_STATE_DISCONNECTED,
};

using client_id = size_t;


class Game_Client {
public:
	Game_Client(ENetPeer* peer, const std::string name, const client_id slot)
		: m_peer(peer), m_name(name), m_slot(slot) {}

	const std::string& name() const { return m_name; }
	const size_t slot() const { return m_slot; }

	void connect() {
		m_state = GAME_CLIENT_STATE_CONNECTED;
	}

	void disconnect() {
		m_state = GAME_CLIENT_STATE_DISCONNECTED;
	}

private:
	Game_Client_State m_state = GAME_CLIENT_STATE_NONE;

	std::string m_name;
	client_id m_slot;
	ENetPeer* m_peer = nullptr;
};

using client_ptr = std::shared_ptr<Game_Client>;

class Game_Client_Manager {
public:
	Game_Client_Manager(std::shared_ptr<spdlog::logger>& logger)
		: m_logger(logger) {}

	client_ptr add_client(const ENetPeer& peer);
	void disconnect_client(const client_id id);

private:
	std::vector<client_ptr> m_clients;

	std::shared_ptr<spdlog::logger> m_logger;
};

