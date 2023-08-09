#include <spdlog/spdlog.h>

#include <enet/enet.h>

#include "state.h"

static std::unordered_map<std::string, ENetPeer*> ClientNames;

void ProcessMessage(ENetHost* server, ENetEvent& event, const Message& message) {
	spdlog::info("Parsing message: Type: '{}' Data: {}",
		MessageTypeString[message.type], message.data);

	switch (message.type) {
	case MessageType_Ping: {
		Message message = {
			MessageType_Pong, "Pong!"
		};

		std::string response = nlohmann::json(message).dump();

		ENetPacket* responsePacket = enet_packet_create(response.c_str(),
			response.size(),
			ENET_PACKET_FLAG_RELIABLE);

		enet_peer_send(event.peer, 0, responsePacket);
		enet_packet_destroy(event.packet);
	} break;

	case MessageType_ListRooms: {

	} break;

	case MessageType_SetClientName: {
		if (ClientNames.find(message.data) != ClientNames.end()) {
			ClientNames[message.data] = event.peer;
		}
	} break;

	default: {
		spdlog::error("Unknown message recieved ({}), exiting", message.type);
		exit(EXIT_FAILURE);
	} break;
	}
}

int main() {
	spdlog::info("Initialising ENet.");
	if (enet_initialize() != 0) {
		spdlog::error("An error occured while initialising Enet.");
		return EXIT_FAILURE;
	}
	atexit(enet_deinitialize);

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = 1234;

	ENetHost* server;
	if (server = enet_host_create(&address, 32, 2, 0); server == NULL) {
		spdlog::error("An error occured while trying to crete an ENet server.");
		return EXIT_FAILURE;
	}

	spdlog::info("Server now running on port: {}.", address.port);
	while (1) {
		ENetEvent event{};
		while (enet_host_service(server, &event, 1000) > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT: {
				spdlog::info("A new client connection from {}:{}.", event.peer->address.host, event.peer->address.port);
			} break;

			case ENET_EVENT_TYPE_RECEIVE: {
				spdlog::info("A packet of length {} containing {} was received from {} on channel {}.",
					event.packet->dataLength,
					(const char*)event.packet->data,
					event.peer->data,
					event.channelID);

				Message message = ParseMessage((char*)event.packet->data, event.packet->dataLength);
				ProcessMessage(server, event, message);
			} break;

			case ENET_EVENT_TYPE_DISCONNECT: {
				spdlog::info("{} disconnected.", event.peer->data);
			} break;

			default: break;
			}
		}
	}

	spdlog::info("Stopping server.");
	enet_host_destroy(server);

	return 0;
}