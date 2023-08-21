#pragma once

#include <nlohmann/json.hpp>
#include <fmt/format.h>

#include <cstdint>
#include <algorithm>

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))


/**
* @brief Generates a client name from a peer.
*
* @param peer The peer to generate a name from.
* @return std::string The generated name.
*/
std::string GenerateClientName(void* peer);

/**
* @brief Creates a client from a peer.
*
* @param peer The peer to create a client from.
* @return Client The created client.
*/
std::string CreateClientFromPeer(void* peer);

enum ClientStatus {
	CLIENT_STATUS_NONE = 0,
	CLIENT_STATUS_CONNECTED,
	CLIENT_STATUS_IN_LOBBY,
	CLIENT_STATUS_IN_GAME,

	CLIENT_STATUS_COUNT,
};

#define CLIENT_STATE_EMPTY -1
#define MAX_CLIENTS_PER_LOBBY 2

using client_id = int32_t;
using lobby_id = int32_t;

struct Lobby {
	std::string name;
	int32_t id;

	std::array<client_id, MAX_CLIENTS_PER_LOBBY> clients = { CLIENT_STATE_EMPTY, CLIENT_STATE_EMPTY };
	std::array<bool, MAX_CLIENTS_PER_LOBBY> ready = { false, false };

	bool isFull() const {
		for (auto& client : clients) {
			if (client == CLIENT_STATE_EMPTY) {
				return false;
			}
		}

		return true;
	}

	bool AreAllClientsReady() {
		return std::all_of(begin(ready), end(ready), [](auto& it) { return it == true; });
	}
};
using LobbyList = std::vector<Lobby>;
inline void to_json(nlohmann::json& json, const Lobby& lobby) {
	auto result = nlohmann::json{
		{"name", lobby.name},
		{"id", lobby.id}
	};

	for (int32_t i = 0; i < MAX_CLIENTS_PER_LOBBY; ++i) {
		result["clients"].push_back(lobby.clients[i]);
		result["ready"].push_back(lobby.ready[i]);
	}

	json = result;
}

inline void from_json(const nlohmann::json& json, Lobby& lobby) {
	lobby.id = json.value("id", -1);
	lobby.name = json.value("name", "");
	if (json.contains("clients")) {
		lobby.clients = json["clients"];
	}

	if (json.contains("ready")) {
		lobby.ready = json["ready"];
	}
}


struct ClientState {
	ClientStatus status = CLIENT_STATUS_NONE;
	client_id id = CLIENT_STATE_EMPTY;
	void* peer = nullptr;
	Lobby* lobby = nullptr;
	int32_t lobbySlot = -1;
	std::string name;
	bool isLocal = false;
	float x;
	float y;
};


struct ServerState {
	LobbyList lobbies;
	size_t activeLobbies = 0;
};

inline nlohmann::json ServerStateToJson(const ServerState& state) {
	nlohmann::json result = {
		{"activeLobbies", state.activeLobbies}
	};

	for (size_t i = 0; i < state.activeLobbies; ++i) {
		nlohmann::json lobby = {
			{"name", state.lobbies[i].name},
			{"id", state.lobbies[i].id}
		};

		for (auto& client : state.lobbies[i].clients) {
			lobby["clients"].push_back(client);
		}

		result["lobbies"].push_back(lobby);
	}

	return result;
}

inline ServerState JsonToServerState(const std::string& jsonStr) {
	ServerState result{};

	if (jsonStr.empty()) {
		return result;
	}

	auto json = nlohmann::json::parse(jsonStr);

	if (json.empty()) {
		return result;
	}

	result.activeLobbies = json.value("activeLobbies", 0);
	for (uint32_t i = 0; i < result.activeLobbies; ++i) {
		Lobby lobby;
		lobby.name = json["lobbies"][i].value("name", "");
		lobby.id = json["lobbies"][i].value("id", 0);

		auto clients = json["lobbies"][i]["clients"];
		for (auto& clientId : clients) {
			lobby.clients[i] = clientId.get<client_id>();
		}

		result.lobbies.push_back(lobby);
	}

	return result;
}

enum PacketType {
	PACKET_TYPE_NONE = 0,

	PACKET_TYPE_PING,
	PACKET_TYPE_PONG,

	PACKET_TYPE_GET_STATE,

	PACKET_TYPE_CLIENT_RECEIVE_NAME,

	PACKET_TYPE_CREATE_LOBBY,
	PACKET_TYPE_JOIN_LOBBY,
	PACKET_TYPE_CLIENT_JOINED_LOBBY,
	PACKET_TYPE_CLIENT_UPDATE_LOBBY,

	PACKET_TYPE_PLAYER_MOVED,
	PACKET_TYPE_SET_READY,
	PACKET_TYPE_GAME_START,

	PACKET_TYPE_COUNT,
};

inline const char* PacketTypeToString(PacketType type) {
	switch (type) {
	case PACKET_TYPE_NONE: return "PACKET_TYPE_NONE";
	case PACKET_TYPE_PING: return "PACKET_TYPE_PING";
	case PACKET_TYPE_PONG: return "PACKET_TYPE_PONG";
	case PACKET_TYPE_GET_STATE: return "PACKET_TYPE_GET_STATE";
	case PACKET_TYPE_CLIENT_RECEIVE_NAME: return "PACKET_TYPE_CLIENT_RECEIVE_NAME";
	case PACKET_TYPE_CREATE_LOBBY: return "PACKET_TYPE_CREATE_LOBBY";
	case PACKET_TYPE_JOIN_LOBBY: return "PACKET_TYPE_JOIN_LOBBY";
	case PACKET_TYPE_CLIENT_JOINED_LOBBY: return "PACKET_TYPE_CLIENT_JOINED_LOBBY";
	case PACKET_TYPE_CLIENT_UPDATE_LOBBY: return "PACKET_TYPE_CLIENT_UPDATE_LOBBY";
	case PACKET_TYPE_PLAYER_MOVED: return "PACKET_TYPE_PLAYER_MOVED";
	case PACKET_TYPE_SET_READY: return "PACKET_TYPE_SET_READY";
	case PACKET_TYPE_GAME_START: return "PACKET_TYPE_GAME_START";
	case PACKET_TYPE_COUNT: return "PACKET_TYPE_COUNT";
	default: assert(false && "unreachable");  return "UNKNOWN";
	}
}

enum PacketProcessedState {
	PACKET_STATE_NONE = 0,
	PACKET_STATE_SUCCESS,
	PACKET_STATE_ERROR,
};

#define MAX_PACKET_DATA_SIZE 1024 * 3

struct Packet {
	PacketType type;
	char data[MAX_PACKET_DATA_SIZE];
	std::string clientId;
	uint32_t timestampMs = 0;
	PacketProcessedState processedState = PACKET_STATE_NONE;
};

inline void SetPacketData(Packet& packet, const nlohmann::json& json) {
	std::string jsonStr = json.dump();
	assert(jsonStr.size() < MAX_PACKET_DATA_SIZE);
	memcpy(packet.data, jsonStr.c_str(), jsonStr.size());
}



