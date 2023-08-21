#include "server.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <enet/enet.h>
#include <nlohmann/json.hpp>

#include <string>
#include <array>
#include <iostream>
#include <list>

#include "state.h"

static ServerState* serverState = nullptr;
static std::shared_ptr<spdlog::logger> logger;
static std::list<ClientState> connectedClients;
static int32_t clientCtr = 0;

void RemoveClient(ClientState* client) {
	if (!client) {
		assert(0 && "unreachable");
		logger->error("Trying to remove NULL client");
	}

	auto it = std::find_if(connectedClients.begin(), connectedClients.end(), [client](const ClientState& c) {
		return c.name == client->name;
		});

	if (it != connectedClients.end()) {
		connectedClients.erase(it);
	}
	else {
		logger->error("Could not remove client from connected clients list, client '{}' not found.", client->name);
	}
}

ClientState* GetClientStateFromID(client_id id) {
	for (auto& client : connectedClients) {
		if (client.id == id) {
			return &client;
		}
	}

	logger->error("Client with ID '{}' not found in connected clients list.", id);
	return nullptr;
}

ClientState* GetClientFromPeer(ENetPeer& peer) {
	for (auto& client : connectedClients) {
		std::string name = GenerateClientName(&peer);
		if (client.name == name) {
			return &client;
		}
	}

	logger->error("Could not get stored client from peer '{}'.", GenerateClientName(&peer));
	assert(0 && "unreachable");
	return nullptr;
}

ClientState* GetClientState(const std::string& name) {
	for (auto& client : connectedClients) {
		if (client.name == name) {
			return &client;
		}
	}

	logger->error("Client '{}' not found in connected clients list.", name);
	return nullptr;
}


void RespondToClient(const Packet& packet, ENetPeer& peer) {
	enet_peer_send(&peer, 0, enet_packet_create(&packet, sizeof(Packet), ENET_PACKET_FLAG_RELIABLE));
	//enet_packet_destroy(event.packet);
}

void RespondToClient(const Packet& packet, ENetEvent& event) {
	RespondToClient(packet, *event.peer);
}

void RespondToClient(const Packet& packet, ClientState& client) {
	RespondToClient(packet, (ENetPeer&)client.peer);
}


void BroadcastToClient(const client_id& id, const Packet& packet) {
	ClientState* state = GetClientStateFromID(id);
	if (state) {
		enet_peer_send((ENetPeer*)state->peer, 0, enet_packet_create(&packet, sizeof(Packet), ENET_PACKET_FLAG_RELIABLE));
	}
	else {
		logger->error("Could not broadcast to client with ID '{}', client not found.", id);
	}
}

void BroadcastToClients(std::vector<client_id>& clients, const Packet& packet) {
	for (auto& client : clients) {
		BroadcastToClient(client, packet);
	}
}
void BroadcastToAllClients(const Packet& packet, const std::vector<client_id>& excludes = {}) {
	for (auto& client : connectedClients) {
		for (auto& exclude : excludes) {
			if (client.id == exclude) {
				continue;
			}
		}

		BroadcastToClient(client.id, packet);
	}
}

void BroadcastToClientsInLobby(const Lobby& lobby, const Packet& packet) {
	for (auto& client : lobby.clients) {
		if (client != -1) {
			logger->info("Broadcasting client {} joined lobby: {}", client, packet.data);
			BroadcastToClient(client, packet);
		}
	}
}

Lobby* AddClientToLobby(ClientState& client, size_t id) {
	for (auto& lobby : serverState->lobbies) {
		if (lobby.id == id) {
			if (lobby.isFull()) {
				return nullptr;
			}

			for (size_t i = 0; i < lobby.clients.size(); ++i) {
				auto& slot = lobby.clients[i];
				if (slot == CLIENT_STATE_EMPTY) {
					slot = client.id;

					logger->info("Adding client to lobby: {} - {}", client.name, lobby.name);
					logger->info("Clients in lobby: {} - {}", lobby.clients[0], lobby.clients[1]);

					client.lobby = &lobby;
					client.lobbySlot = i;

					return &lobby;
				}
			}
		}
	}

	return nullptr;
}

Lobby& AddLobby(const std::string& name) {
	logger->info("Creating lobby {}: {}", serverState->activeLobbies, name);

	Lobby lobby;
	lobby.name = name;
	lobby.id = serverState->lobbies.size();

	for (auto& client : lobby.clients) {
		client = CLIENT_STATE_EMPTY;
	}

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
	if (packet.type != PACKET_TYPE_PING) {
		logger->info("Parsing packet: Type: '{}' Data: {}",
			PacketTypeToString(packet.type), packet.data);
	}

	if (packet.type == PACKET_TYPE_GET_STATE) {
		// Update the packet for the return trip.
		auto json = ServerStateToJson(*serverState).dump();
		if (json.size() > sizeof(packet.data)) {
			logger->error("Server state is too large to fit in packet, exiting.");
			exit(EXIT_FAILURE);
		}
		strcpy_s(packet.data, sizeof(packet.data), json.c_str());
		ClientState* client = GetClientFromPeer(*event.peer);
		if (client) {
			packet.clientId = client->name;
		}
	}

	switch (packet.type) {
	case PACKET_TYPE_PING: {
		packet.type = PACKET_TYPE_PONG;

		strcpy_s(packet.data, sizeof(packet.data), "Pong!");
		packet.processedState = PACKET_STATE_SUCCESS;
		RespondToClient(packet, event);
	} break;

	case PACKET_TYPE_CREATE_LOBBY: {
		Lobby& lobby = AddLobby(packet.data);
		logger->info("Created lobby: {}", lobby.name);
		logger->info("Clients in lobby: {} - {}", lobby.clients[0], lobby.clients[1]);
		packet.processedState = PACKET_STATE_SUCCESS;
		RespondToClient(packet, event);
	} break;

	case PACKET_TYPE_PLAYER_MOVED: {
		packet.type = PACKET_TYPE_PLAYER_MOVED;

		nlohmann::json data = nlohmann::json::parse(packet.data);
		strcpy_s(packet.data, sizeof(packet.data), data.dump().c_str());

		auto client = GetClientFromPeer(*event.peer);
		BroadcastToClientsInLobby(*client->lobby, packet);
	} break;

	case PACKET_TYPE_SET_READY: {
		auto client = GetClientFromPeer(*event.peer);
		if (client) {
			bool ready = (strcmp(packet.data, "true") == 0);

			logger->info("Client {} is ready: {}", client->name, ready);
			client->lobby->ready[client->lobbySlot] = ready;

			nlohmann::json json = {
				{"isLocalPlayer", true},
				{"lobbyIdx", client->lobbySlot}
			};
			strcpy_s(packet.data, sizeof(packet.data), json.dump().c_str());
			RespondToClient(packet, event);

			if (client->lobby->isFull() && client->lobby->AreAllClientsReady()) {
				packet.type = PACKET_TYPE_GAME_START;
				nlohmann::json positions;
				nlohmann::json posA = { {"x", 100},{"y", 100}, };
				nlohmann::json posB = { {"x", 500},{"y", 500}, };
				positions.push_back(posA);
				positions.push_back(posB);

				auto data = positions.dump();
				strcpy_s(packet.data, sizeof(packet.data), data.c_str());

				BroadcastToClientsInLobby(*client->lobby, packet);
				logger->info("Starting game for lobby: {}. Initial state; {}", client->lobby->name, data);
			}
		}
		else {
			logger->error("Could not set client ready, client not found.");
		}
	} break;

	case PACKET_TYPE_CLIENT_UPDATE_LOBBY: {
		auto* client = GetClientFromPeer(*event.peer);
		assert(client && client->lobby);

		nlohmann::json lobby;
		to_json(lobby, *client->lobby);
		SetPacketData(packet, lobby);
		RespondToClient(packet, event);
	} break;

	case PACKET_TYPE_JOIN_LOBBY: {
		Lobby* lobby = nullptr;
		const auto client = GetClientFromPeer(*event.peer);

		int32_t slot = -1;
		// Respond back to client with the lobby id.
		{
			size_t id = std::stoi(packet.data);
			if (client) {
				if (lobby = AddClientToLobby(*client, id); lobby != nullptr) {
					for (size_t i = 0; i < MAX_CLIENTS_PER_LOBBY; ++i) {
						if (lobby->clients[i] == client->id) {
							slot = i;
							break;
						}
					}
				}
			}
		}

		// Broadcast to all connected clients to the lobby that a new client has joined.
		if (lobby) {
			Packet packet = {};
			packet.type = PACKET_TYPE_CLIENT_JOINED_LOBBY;

			nlohmann::json json({
				{"client", client->id},
				{"clientName", client->name},
				{"lobby", lobby->id},
				{"slot", slot},
				});

			SetPacketData(packet, json);
			BroadcastToClientsInLobby(*lobby, packet);
		}
	} break;

	case PACKET_TYPE_GET_STATE: {
		packet.processedState = PACKET_STATE_SUCCESS;
		RespondToClient(packet, event);
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
				ClientState client;
				client.name = GenerateClientName(event.peer);
				client.peer = (void*)event.peer;
				client.isLocal = true;
				client.id = clientCtr++;

				logger->info("Adding client {} to connected clients.", client.name);
				connectedClients.push_back(client);

				Packet response = {};
				response.type = PACKET_TYPE_CLIENT_RECEIVE_NAME;
				strcpy_s(response.data, sizeof(response.data), client.name.c_str());
				response.processedState = PACKET_STATE_SUCCESS;
				RespondToClient(response, event);
			} break;

			case ENET_EVENT_TYPE_RECEIVE: {
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
				logger->info("Disconnecting client {} from connected clients.", GenerateClientName((void*)event.peer));
				RemoveClient(GetClientFromPeer(*event.peer));
			} break;

			default: break;
			}
		}
	}

}

int main() {
	serverState = new ServerState;
	//logger = logger->rotating_logger_mt("rotating_logger", "logs/server.log", 1048576 * 5, 3);
	logger = spdlog::stdout_color_mt("SERVER");

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

#if 0
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
			fmt::print("{}\n", ServerStateToJson(*serverState).dump(2));
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
#else
	ServerThread();
#endif

	logger->info("Stopping server.");

	spdlog::shutdown();
	enet_host_destroy(server);

	return 0;
}