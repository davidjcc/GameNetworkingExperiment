#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <enet/enet.h>
#include <nlohmann/json.hpp>

#include <string>
#include <array>
#include <iostream>

#define JSON_SERIALISE_DEFINE
#include "state.h"

static ServerState* serverState = nullptr;
static std::shared_ptr<spdlog::logger> logger;

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

	logger->error("Client '{}' not found in connected clients list.", name);
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

Lobby& AddLobby(const std::string& name) {
	Lobby lobby;
	lobby.name = name;
	lobby.id = serverState->lobbies.size();

	serverState->lobbies.push_back(lobby);
	serverState->activeLobbies++;

	return serverState->lobbies[serverState->lobbies.size() - 1];
}

void RemoveLobby(const std::string& name) {
	for (size_t i = 0; i < serverState->activeLobbies; ++i) {
		if (serverState->lobbies[i].name == name) {
			// Remove lobby from active lobbies.
			serverState->lobbies.erase(serverState->lobbies.begin() + i);
			serverState->activeLobbies--;
			break;
		}
	}
}

void ProcessMessage(ENetHost& server, ENetEvent& event, const Message& message) {
	logger->info("Parsing message: Type: '{}' Data: {}",
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

		lobby = AddLobby(lobby.name);

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
	default: logger->error("Unknown message recieved ({} - {}), exiting", message.type, message.data); break;
	}

	//enet_packet_destroy(event.packet);
}

static ENetHost* server;
static bool killServer = false;

void ServerThread() {

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = 1234;

	if (server = enet_host_create(&address, 32, 2, 0); server == NULL) {
		logger->error("An error occured while trying to crete an ENet server.");
		exit(EXIT_FAILURE);
	}

	logger->info("Server now running on port: {}.", address.port);
	while (!killServer) {
		ENetEvent event{};
		while (enet_host_service(server, &event, 1000) > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT: {
				Client client = CreateClientFromPeer(event.peer);

				logger->info("Adding client {} to connected clients.", client.name);
				serverState->connectedClients.push_back(client);

				RespondToClient({ MessageType_ClientReceiveName, client.name }, *server, event);
			} break;

			case ENET_EVENT_TYPE_RECEIVE: {
				std::string packetStr((char*)event.packet->data, event.packet->dataLength);
				logger->info("A packet of length {} containing '{}' was received from {} on channel {}.",
					event.packet->dataLength,
					packetStr,
					event.peer->data,
					event.channelID);

				Message message = ParseMessage((char*)event.packet->data, event.packet->dataLength);
				ProcessMessage(*server, event, message);
			} break;

			case ENET_EVENT_TYPE_DISCONNECT: {
				logger->info("Disconnecting client {} from connected clients.", GetClientFromPeer(*event.peer).name);
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

}

int main() {
	serverState = new ServerState;
	logger = spdlog::rotating_logger_mt("rotating_logger", "logs/server.log", 1048576 * 5, 3);

	if (!logger) {
		fmt::print("Error creating logger\n");
		exit(EXIT_FAILURE);
	}

	logger->info("Initialising ENet.");
	if (enet_initialize() != 0) {
		logger->error("An error occured while initialising Enet.");
		return EXIT_FAILURE;
	}
	atexit(enet_deinitialize);

	std::thread serverThread(ServerThread);


	enum CommandType {
		CommandType_Exit,
		CommandType_State,
		CommandType_Help,
		CommandType_AddLobby,
		CommandType_RemoveLobby,
	};

	struct Command {
		CommandType type;
		std::string help;
	};

	std::unordered_map<std::string, Command> commands = {
		{"exit", {CommandType_Exit, "Kill the server and exit the program"}},
		{"state", {CommandType_State, "Output the current server state"}},
		{"help", {CommandType_Help, "Display all available commands"}},
		{"lobby_add", {CommandType_AddLobby, "Add a new lobby"}},
		{"lobby_remove", {CommandType_RemoveLobby, "Remove a lobby"}},
	};


	AddLobby("test1");
	AddLobby("test2");
	AddLobby("test3");
	AddLobby("test5");

	std::string commandInput;
	while (!killServer) {
		fmt::print("Enter a command: ");
		std::getline(std::cin, commandInput);

		if (commands.find(commandInput) == commands.end()) {
			fmt::print("Unknown command: {}\n", commandInput);
			continue;
		}

		CommandType command = commands[commandInput].type;
		switch (command) {
		case CommandType_Exit: killServer = true; break;
		case CommandType_State: {
			fmt::print("{}\n", serverState->ToJsonString(2));
		} break;

		case CommandType_Help: {
			for (auto& [str, command] : commands) {
				fmt::print("{} - {}\n", str, command.help);
			}
		} break;

		case CommandType_AddLobby: {
			std::string lobbyName;
			fmt::print("Enter a lobby name: ");
			std::getline(std::cin, lobbyName);

			AddLobby(lobbyName);
		} break;

		case CommandType_RemoveLobby: {
				std::string lobbyName;
				fmt::print("Enter a lobby name: ");
				std::getline(std::cin, lobbyName);

				RemoveLobby(lobbyName);
			} break;

		default: {
			fmt::print("Unknown command: {}\n", commandInput);
		} break;

		}
	}

	serverThread.join();

	logger->info("Stopping server.");

	spdlog::shutdown();
	enet_host_destroy(server);

	return 0;
}