#pragma once

#include <nlohmann/json.hpp>
#include <fmt/format.h>

#include <cstdint>

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))


enum ClientStatus {
	ClientStatus_None = 0,
	ClientStatus_Connected,
	ClientStatus_InLobby,
	ClientStatus_InGame,

	ClientStatus_Count,
};

static const char* ClientStatusString[ClientStatus_Count] = {
	"ClientStatus_None",
	"ClientStatus_Connected",
	"ClientStatus_InLobby",
	"ClientStatus_InGame",
};
static_assert(ARRAY_COUNT(ClientStatusString) == ClientStatus_Count, "ClientStatusString does not match ClientStatus_Count");

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

#define MAX_CLIENTS_PER_LOBBY 2
struct Lobby {
	std::string name;
	size_t id;

	std::array<std::string, MAX_CLIENTS_PER_LOBBY> clients;
};
using LobbyList = std::vector<Lobby>;

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
			{"id", state.lobbies[i].id},
			{"clients", state.lobbies[i].clients}
		};

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
		lobby.clients = json["lobbies"][i].value("clients", std::array<std::string, MAX_CLIENTS_PER_LOBBY>());

		result.lobbies.push_back(lobby);
	}

	return result;
}

enum PacketType {
	PACKET_TYPE_NONE = 0,

	PACKET_TYPE_PING,
	PACKET_TYPE_PONG,

	PACKET_TYPE_CLIENT_RECEIVE_NAME,

	PACKET_TYPE_CREATE_LOBBY,
	PACKET_TYPE_JOIN_LOBBY,

	PACKET_TYPE_COUNT,
};

inline const char* PacketTypeToString(PacketType type) {
	switch (type) {
	case PACKET_TYPE_NONE: return "PACKET_TYPE_NONE";
	case PACKET_TYPE_PING: return "PACKET_TYPE_PING";
	case PACKET_TYPE_PONG: return "PACKET_TYPE_PONG";
	case PACKET_TYPE_CLIENT_RECEIVE_NAME: return "PACKET_TYPE_CLIENT_RECEIVE_NAME";
	case PACKET_TYPE_CREATE_LOBBY: return "PACKET_TYPE_CREATE_LOBBY";
	case PACKET_TYPE_JOIN_LOBBY: return "PACKET_TYPE_JOIN_LOBBY";
	default: assert(0 && "unreachable");  return "Unknown PacketType";
	}
}

struct Packet {
	PacketType type;
	std::string data;
	std::string clientId;
	uint32_t timestampMs = 0;
	char state[1024 * 3];
};



