#pragma once

#include <nlohmann/json.hpp>

enum MessageType {
	MessageType_None = 0,

	MessageType_Ping,
	MessageType_Pong,

	MessageType_SetClientName,
	MessageType_ListRooms,

	MessageType_Success,
	MessageType_Failure,

	MessageType_Count,
};

static const char* MessageTypeString[MessageType_Count] = {
	"MessageType_None",

	"MessageType_Ping",
	"MessageType_Pong",

	"MessageType_SetClientName",
	"MessageType_ListRooms"

	"MessageType_Success"
	"MessageType_Failure"
};


struct Message {
	MessageType type;

	std::string data;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Message, type, data);

inline Message ParseMessage(const char* data, size_t dataLength) {
	return nlohmann::json::parse(std::string((char*)data, dataLength));
}

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

