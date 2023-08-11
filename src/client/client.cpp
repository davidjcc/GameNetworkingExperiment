#include "client.h"
#include "state.h"

#include <enet/enet.h>
#include <spdlog/spdlog.h>
#include <iostream>

static ENetAddress address;
static ENetHost* client;
static ENetPeer* peer;
static int eventStatus;
static bool connected = false;

static std::string clientName;

namespace Net {
	bool IsConnected() { return connected; }

	std::string GetClientName() { return clientName; }

	bool InitClient() {
		if (enet_initialize() != 0) {
			spdlog::error("An error occured while initializing ENet.");
			return false;
		}

		atexit(enet_deinitialize);

		client = enet_host_create(NULL, 1, 2, 0);

		if (client == NULL) {
			spdlog::error("An error occured while trying to create an ENet server host");
			return false;
		}

		enet_address_set_host(&address, "localhost");
		address.port = 1234;

		peer = enet_host_connect(client, &address, 2);

		if (peer == NULL) {
			spdlog::error("No available peers for initializing an ENet connection");
			return false;
		}

		return true;
	}

	Message PollServer() {
		Message message = { MessageType_None };

		ENetEvent event;

		// If we had some event that interested us
		while (enet_host_service(client, &event, 0)) {
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT: {
				spdlog::info("We got a new connection from {}", event.peer->address.host);
				connected = true;
			} break;

			case ENET_EVENT_TYPE_RECEIVE: {
				spdlog::info("Message recieved from server {}: '{}'",
					event.peer->address.host,
					std::string((const char*)event.packet->data, event.packet->dataLength));

				message = ParseMessage((const char*)event.packet->data, event.packet->dataLength);
				spdlog::info("Parsed mesaage type: {} data: {}", MessageTypeString[message.type], message.data);

				switch (message.type) {
				case MessageType_ClientReceiveName: {
					clientName = message.data;
				} break;
				case MessageType_Pong:
					break;

				default: break;
				}

				enet_packet_destroy(event.packet);
			} break;

			case ENET_EVENT_TYPE_DISCONNECT: {
				spdlog::info("{} disconnected.", (const char*)event.peer->data);
				event.peer->data = NULL;
			} break;
			}
		}

		return message;
	}

	void SendRequestToServer(const Message& message) {
		std::string msgString = nlohmann::json(message).dump();

		ENetPacket* packet = enet_packet_create(msgString.c_str(), msgString.size(), ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, packet);

	}

	void Shutdown() {
		enet_peer_disconnect(peer, 0);

		// Wait up to 3 seconds for the disconnect to complete gracefully
		ENetEvent event;
		while (enet_host_service(client, &event, 3000) > 0) {
			if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
				fmt::print("Disconnection succeeded.\n");
				break;
			}
		}

		enet_peer_reset(peer);
		enet_host_destroy(client);
	}

	namespace Request {
		void Ping() {
			if (connected) {
				SendRequestToServer({ MessageType_Ping });
			}
			else {
				InitClient();
			}
		}

		void ListLobbies() {
			if (connected) {
				SendRequestToServer({ MessageType_ListLobbies });
			}
			else {
				InitClient();
			}
		}
	}

}
