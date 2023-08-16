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
static std::vector<std::string> connectedClients;

std::string GetClientFromPeer(ENetPeer& peer) {
	for (std::string& client : connectedClients) {
		std::string name = GenerateClientName(&peer);
		if (client == name) {
			return client;
		}
	}

	return CreateClientFromPeer(&peer);
}

std::string GetConnectedClient(const std::string& name) {
	for (std::string client : connectedClients) {
		if (client == name) {
			return client;
		}
	}

	logger->error("Client '{}' not found in connected clients list.", name);
	exit(EXIT_FAILURE);
}

void RespondToClient(const Packet& packet, ENetHost& server, ENetEvent& event) {
	enet_peer_send(event.peer, 0, enet_packet_create(&packet, sizeof(Packet), ENET_PACKET_FLAG_RELIABLE));
	//enet_packet_destroy(event.packet);
}

void AddClientToLobby(const std::string& client, size_t id) {
	for (auto& lobby : serverState->lobbies) {
		if (lobby.id == id) {
			for (auto& slot : lobby.clients) {
				if (slot.empty()) {
					slot = client;
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

void ProcessMessage(ENetHost& server, ENetEvent& event, Packet& packet) {
	logger->info("Parsing packet: Type: '{}' Data: {}",
		PacketTypeToString(packet.type), packet.data);

	// Update the packet for the return trip.
	auto json = ServerStateToJson(*serverState).dump();
	if (json.size() > sizeof(packet.state)) {
		logger->error("Server state is too large to fit in packet, exiting.");
		exit(EXIT_FAILURE);
	}
	strcpy(packet.state, json.c_str());
	packet.clientId = GetClientFromPeer(*event.peer);

	switch (packet.type) {
	case PACKET_TYPE_PING: {
		packet.type = PACKET_TYPE_PONG;
		packet.data = "Pong!";
		RespondToClient(packet, server, event);
	} break;

	case PACKET_TYPE_CREATE_LOBBY: {
		AddLobby(packet.data);
		RespondToClient(packet, server, event);
	} break;

	case PACKET_TYPE_JOIN_LOBBY: {
		size_t id = std::stoi(packet.data);
		const auto client = GetClientFromPeer(*event.peer);
		AddClientToLobby(GetConnectedClient(client), id);
		RespondToClient(packet, server, event);
	} break;

	default: {
		logger->error("Unknown packet recieved ({} - {}), exiting", PacketTypeToString(packet.type), packet.data);
	} break;
	}
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
				auto client = CreateClientFromPeer(event.peer);

				logger->info("Adding client {} to connected clients.", client);
				connectedClients.push_back(client);
				RespondToClient({ PACKET_TYPE_CLIENT_RECEIVE_NAME, client }, *server, event);
			} break;

			case ENET_EVENT_TYPE_RECEIVE: {
				std::string packetStr((char*)event.packet->data, event.packet->dataLength);
				logger->info("A packet of length {} containing '{}' was received from {} on channel {}.",
					event.packet->dataLength, packetStr, event.peer->data, event.channelID);

				if (event.packet->dataLength == sizeof(Packet)) {
					Packet packet = *(Packet*)event.packet->data;
					ProcessMessage(*server, event, packet);
				}
				else {
					logger->error("Error parsing packet, packet size is not equal to sizeof(Message).");
				}

				enet_packet_destroy(event.packet);
			} break;

			case ENET_EVENT_TYPE_DISCONNECT: {
				logger->info("Disconnecting client {} from connected clients.", GetClientFromPeer(*event.peer));
				connectedClients.erase(std::remove_if(connectedClients.begin(), connectedClients.end(),
					[&](const std::string& client) {
						return client == std::to_string(event.peer->address.host);
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
		CommandType_RemoveAllLobbies,
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
		{"lobby_clear", {CommandType_RemoveAllLobbies, "Remove all lobbies"}},
	};

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
			// TODO(DC): Print out the current server state.
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

		case CommandType_RemoveAllLobbies: {
			for (auto& lobby : serverState->lobbies)
				RemoveLobby(lobby.name);
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