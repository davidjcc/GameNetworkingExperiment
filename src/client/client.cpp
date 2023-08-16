#include "client.h"

#include <spdlog/spdlog.h>

#include "state.h"

#include <enet/enet.h>
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
			//spdlog::error("An error occured while initializing ENet.");
			return false;
		}

		atexit(enet_deinitialize);

		client = enet_host_create(NULL, 1, 2, 0);

		if (client == NULL) {
			//spdlog::error("An error occured while trying to create an ENet server host");
			return false;
		}

		enet_address_set_host(&address, "localhost");
		address.port = 1234;

		peer = enet_host_connect(client, &address, 2);

		if (peer == NULL) {
			//spdlog::error("No available peers for initializing an ENet connection");
			return false;
		}

		return true;
	}

	void PollServer(PollCallback cb) {
		ENetEvent event;

		// If we had some event that interested us
		while (enet_host_service(client, &event, 0)) {
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT: {
				//spdlog::info("We got a new connection from {}", event.peer->address.host);
				connected = true;
			} break;

			case ENET_EVENT_TYPE_RECEIVE: {
				//spdlog::info("Message recieved from server {}: '{}'",
					//event.peer->address.host,
					//std::string((const char*)event.packet->data, event.packet->dataLength));

				if (event.packet->dataLength == sizeof(Packet)) {
					Packet* packet = (Packet*)event.packet->data;
					if (packet) {
						switch (packet->type) {
						case PACKET_TYPE_CLIENT_RECEIVE_NAME: {
							clientName = packet->data;
						} break;

						case PACKET_TYPE_PONG:
						default: break;
						}

						cb(*packet);
					}
				}
				else {
					// TODO(DC): error msg.
				}

				//spdlog::info("Parsed mesaage type: {} data: {}", MessageTypeString[message.type], message.data);
				//enet_packet_destroy(event.packet);
			} break;

			case ENET_EVENT_TYPE_DISCONNECT: {
				//spdlog::info("{} disconnected.", (const char*)event.peer->data);
				event.peer->data = NULL;
			} break;
			}
		}
	}

	void SendRequestToServer(const Packet& packet) {
		enet_peer_send(peer, 0, enet_packet_create(&packet, sizeof(Packet), ENET_PACKET_FLAG_RELIABLE));
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
#define SEND_REQUEST(PACKET) \
	if (connected) { \
		SendRequestToServer(PACKET); \
	}\
	else { \
		InitClient(); \
	} 

		void Ping(const uint32_t timestamp) {
			Packet packet = {
				.type = PACKET_TYPE_PING,
				.data = "",
				.timestampMs = timestamp
			};
			SEND_REQUEST(packet);
		}

		void CreateLobby(const std::string& name) {
			Packet packet = {
				.type = PACKET_TYPE_CREATE_LOBBY,
				.data = name,
			};
			SEND_REQUEST(packet);
		}

		void JoinLobby(const size_t id) {
			Packet packet = {
				.type = PACKET_TYPE_JOIN_LOBBY,
				.data = std::to_string(id),
			};
			SEND_REQUEST(packet);
		}
	}

}
