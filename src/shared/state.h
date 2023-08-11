#pragma once

#include <nlohmann/json.hpp>

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

enum MessageType {
	MessageType_None = 0,

	MessageType_Ping,
	MessageType_Pong,

	MessageType_ClientReceiveName,

	MessageType_ListLobbies,

	MessageType_Success,
	MessageType_Failure,

	MessageType_Count,
};

static const char* MessageTypeString[MessageType_Count] = {
	"MessageType_None",

	"MessageType_Ping",
	"MessageType_Pong",

	"MessageType_ClientReceiveName",

	"MessageType_ListLobbies"

	"MessageType_Success"
	"MessageType_Failure"
};
static_assert(ARRAY_COUNT(MessageTypeString) == MessageType_Count, "MessageTypeString does not match MessageType_Count");

struct Message {
	MessageType type;

	std::string data;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Message, type, data);

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
	ClientStatus_Connected = 0,
	ClientStatus_InLobby,
	ClientStatus_InGame,

	ClientStatus_Count,
};

static const char* ClientStatusString[ClientStatus_Count] = {
	"ClientStatus_Connected",
	"ClientStatus_InLobby",
	"ClientStatus_InGame",
};
static_assert(ARRAY_COUNT(ClientStatusString) == ClientStatus_Count, "ClientStatusString does not match ClientStatus_Count");

struct Client {
	ClientStatus status;
	std::string name;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Client, status, name);

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

#define MAX_CLIENTS_PER_LOBBY 2
struct Lobby {
	std::string name;
	size_t id;

	std::array<Client*, MAX_CLIENTS_PER_LOBBY> clients;
};
void to_json(nlohmann::json& nlohmann_json_j, const Lobby& nlohmann_json_t);
void from_json(const nlohmann::json& nlohmann_json_j, Lobby& nlohmann_json_t);

using LobbyList = std::vector<Lobby>;

struct ServerState {
	LobbyList lobbies;
	size_t activeLobbies = 0;

	std::vector<Client> connectedClients;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ServerState, lobbies, activeLobbies);


