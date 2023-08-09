#include "client.h"
#include "state.h"

#include <enet/enet.h>
#include <spdlog/spdlog.h>
#include <iostream>

static ENetAddress address;
static ENetHost* client;
static ENetPeer* peer;
static ENetEvent event;
static int eventStatus;
static bool connected = false;

namespace Net {
	bool IsConnected() { return connected; }

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


	Message Poll() {
		Message message = { MessageType_None };

		eventStatus = enet_host_service(client, &event, 0);

		// If we had some event that interested us
		if (eventStatus > 0) {
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

	void Ping() {
		if (connected) {
			Message msg = {
				MessageType_Ping, ""
			};
			std::string msgString = nlohmann::json(msg).dump();

			ENetPacket* packet = enet_packet_create(msgString.c_str(), msgString.size(), ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(peer, 0, packet);
		}
	}

	void Shutdown() {
		enet_peer_reset(peer);
		enet_host_destroy(client);
	}
}
