#include <spdlog/spdlog.h>
#include <enet/enet.h>
#include <nlohmann/json.hpp>

#include <string>
#include <array>

#include "state.h"

static ServerState* serverState = nullptr;

Client GetClientFromPeer(ENetPeer& peer) {
	for (Client& client : serverState->connectedClients) {
		std::string name = GenerateClientName(&peer);
		if (client.name == name) {
			return client;
		}
	}

	return CreateClientFromPeer(&peer);
}

void RespondToClient(const Message& message, ENetHost& server, ENetEvent& event) {
	std::string response = nlohmann::json(message).dump();

	ENetPacket* responsePacket = enet_packet_create(response.c_str(),
		response.size(),
		ENET_PACKET_FLAG_RELIABLE);

	enet_peer_send(event.peer, 0, responsePacket);
	//enet_packet_destroy(event.packet);
}

void ProcessMessage(ENetHost& server, ENetEvent& event, const Message& message) {
	spdlog::info("Parsing message: Type: '{}' Data: {}",
		MessageTypeString[message.type], message.data);

	switch (message.type) {
	case MessageType_Ping: RespondToClient({ MessageType_Pong, "Pong!" }, server, event); break;
	case MessageType_ListLobbies: RespondToClient({ message.type, nlohmann::json(*serverState).dump() }, server, event); break;
	default: spdlog::error("Unknown message recieved ({} - {}), exiting", message.type, message.data); break;
	}

	//enet_packet_destroy(event.packet);
}

int main() {
	serverState = new ServerState;

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
				Client client = CreateClientFromPeer(event.peer);

				spdlog::info("Adding client {} to connected clients.", client.name);
				serverState->connectedClients.push_back(client);

				RespondToClient({ MessageType_ClientReceiveName, client.name }, *server, event);
			} break;

			case ENET_EVENT_TYPE_RECEIVE: {
				std::string packetStr((char*)event.packet->data, event.packet->dataLength);
				spdlog::info("A packet of length {} containing '{}' was received from {} on channel {}.",
					event.packet->dataLength,
					packetStr,
					event.peer->data,
					event.channelID);

				Message message = ParseMessage((char*)event.packet->data, event.packet->dataLength);
				ProcessMessage(*server, event, message);
			} break;

			case ENET_EVENT_TYPE_DISCONNECT: {
				spdlog::info("Disconnecting client {} from connected clients.", event.peer->data);
				auto& connectedClients = serverState->connectedClients;
				connectedClients.erase(std::remove_if(connectedClients.begin(), connectedClients.end(),
					[&](const Client& client) {
						return client.name == std::to_string(event.peer->address.host);
					}), connectedClients.end());
			} break;

			default: break;
			}
		}
	}

	spdlog::info("Stopping server.");
	enet_host_destroy(server);

	return 0;
}