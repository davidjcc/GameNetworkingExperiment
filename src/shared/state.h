#pragma once

#include <nlohmann/json.hpp>
#include <fmt/format.h>

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

enum MessageType {
	MessageType_None = 0,

	MessageType_Ping,
	MessageType_Pong,

	MessageType_ClientReceiveName,

	MessageType_ListLobbies,
	MessageType_CreateLobby,
	MessageType_JoinLobby,

	MessageType_Success,
	MessageType_Failure,

	MessageType_Count,
};

static const char* MessageTypeString[MessageType_Count] = {
	"MessageType_None",

	"MessageType_Ping",
	"MessageType_Pong",

	"MessageType_ClientReceiveName",

	"MessageType_ListLobbies",
	"MessageType_CreateLobby",
	"MessageType_JoinLobby",

	"MessageType_Success",
	"MessageType_Failure"
};
static_assert(ARRAY_COUNT(MessageTypeString) == MessageType_Count, "MessageTypeString does not match MessageType_Count");

struct Message {
	MessageType type;
	std::string data;
	std::string clientId;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Message, type, data, clientId);

/**
* @brief Parses a message from a string.
*
* @param data The string to parse.
* @return Message The parsed message.
*/
Message ParseMessage(const char* data, size_t dataLength);

struct Player {
	float x, y, width, height;
	bool isAi = false;
	float score;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Player, x, y, width, height, isAi, score);

struct Ball {
	float x, y, width, height;
	float velX, velY;
	float speed;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Ball, x, y, width, height, velX, velY, speed);

struct State {
	Player left;
	Player right;
	Ball ball = {};
	float resetTimer = 0.0f;
	bool isDemo = false;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(State, left, right, ball, resetTimer);

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

struct Client {
	ClientStatus status = ClientStatus_None;
	std::string name;

	std::string ToJsonString(int indent = -1) {
		auto json = ToJson();
		return json.dump(indent);
	}


	nlohmann::json ToJson() {
		return {
			{"clientStatus", status},
			{"name", name}
		};
	}

	void FromJson(const nlohmann::json& json) {
		status = json.value("clientStatus", ClientStatus_Connected);
		name = json.value("name", "empty");
	}
};


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
Client CreateClientFromPeer(void* peer);

struct InitLobby {
	std::string name;

	std::string ToJsonString(int indent = -1) {
		auto json = ToJson();
		return json.dump(indent);
	}


	nlohmann::json ToJson() {
		return {
			{"name", name}
		};
	}

	void FromJson(const nlohmann::json& json) {
		name = json.value("name", "empty");
	}
};

#define MAX_CLIENTS_PER_LOBBY 2
struct Lobby {
	Lobby() {
		for (size_t i = 0; i < MAX_CLIENTS_PER_LOBBY; ++i) {
			clients[i] = nullptr;
		}
	}

	std::string name;
	size_t id;

	std::array<Client*, MAX_CLIENTS_PER_LOBBY> clients;

	std::string ToJsonString(int indent = -1) {
		auto json = ToJson();
		return json.dump(indent);
	}

	nlohmann::json ToJson() {
		return {
			{"name", name},
			{"id", id}
		};
	}

	void FromJson(const nlohmann::json& json) {
		name = json.value("name", "empty");
		id = json.value("id", 0);
	}
};
using LobbyList = std::vector<Lobby>;

struct ServerState {
	LobbyList lobbies;
	size_t activeLobbies = 0;

	std::vector<Client> connectedClients;

	std::string ToJsonString(int indent = -1) {
		auto json = ToJson();
		return json.dump(indent);
	}


	nlohmann::json ToJson() {
		nlohmann::json result = {
			{"activeLobbies", activeLobbies}
		};
		for (size_t i = 0; i < activeLobbies; ++i) {
			result["lobbies"].push_back(lobbies[i].ToJson());
		}
		return result;
	}

	void FromJson(const nlohmann::json& json) {
		auto lobbiesJson = json.contains("lobbies") ? json["lobbies"] : nlohmann::json();
		for (auto& lobbyJson : lobbiesJson) {
			Lobby lobby;
			lobby.FromJson(lobbyJson);
			lobbies.push_back(lobby);
		}

		activeLobbies = json.value("activeLobbies", 0);

		auto connectedClientsJson = json.contains("connectedClients") ? json["connectedClients"] : nlohmann::json();
		for (auto& clientJson : connectedClientsJson) {
			Client client;
			client.FromJson(clientJson);
			connectedClients.push_back(client);
		}
	}
};



