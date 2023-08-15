#include <spdlog/spdlog.h>
#include <enet/enet.h>
#include <nlohmann/json.hpp>

#include <string>
#include <array>

#define JSON_SERIALISE_DEFINE
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

Client& GetConnectedClient(const std::string& name) {
	for (Client& client : serverState->connectedClients) {
		if (client.name == name) {
			return client;
		}
	}

	spdlog::error("Client '{}' not found in connected clients list.", name);
	exit(EXIT_FAILURE);
}

void RespondToClient(const Message& message, ENetHost& server, ENetEvent& event) {
	std::string response = nlohmann::json(message).dump();

	ENetPacket* responsePacket = enet_packet_create(response.c_str(),
		response.size(),
		ENET_PACKET_FLAG_RELIABLE);

	enet_peer_send(event.peer, 0, responsePacket);
	//enet_packet_destroy(event.packet);
}

void AddClientToLobby(Client& client, size_t id) {
	for (auto& lobby : serverState->lobbies) {
		if (lobby.id == id) {
			for (auto& slot : lobby.clients) {
				if (slot == nullptr) {
					slot = &client;
				}
			}
		}
	}
}

void ProcessMessage(ENetHost& server, ENetEvent& event, const Message& message) {
	spdlog::info("Parsing message: Type: '{}' Data: {}",
		MessageTypeString[message.type], message.data);

	switch (message.type) {
	case MessageType_Ping: RespondToClient({ MessageType_Pong, "Pong!" }, server, event); break;
	case MessageType_ListLobbies: {
		Message response = {
			.type = MessageType_ListLobbies,
			.data = serverState->ToJsonString(),
			.clientId = GetClientFromPeer(*event.peer).name,
		};
		RespondToClient(response, server, event);
	} break;

	case MessageType_CreateLobby: {
		Lobby lobby;
		lobby.FromJson(nlohmann::json::parse(message.data));

		Client client = GetClientFromPeer(*event.peer);

		serverState->lobbies.push_back(lobby);
		serverState->activeLobbies++;

		RespondToClient({ message.type, lobby.ToJsonString() }, server, event);
	} break;

	case MessageType_JoinLobby: {
		size_t id = std::stoi(message.data);

		const auto client = GetClientFromPeer(*event.peer);

		AddClientToLobby(GetConnectedClient(client.name), id);

		// Respond with a list lobbies message to update the client's state.
		// TODO(DC): Rename list lobbies message type to update server state or something.
		ProcessMessage(server, event, { MessageType_ListLobbies });
	} break;
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
				spdlog::info("Disconnecting client {} from connected clients.", GetClientFromPeer(*event.peer).name);
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