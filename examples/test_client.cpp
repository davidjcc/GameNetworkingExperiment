#include "server.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>

#include "enet.h"

const char* host = "localhost";
int32_t port = 1234;

std::shared_ptr<spdlog::logger> logger;

int main() {
	logger = spdlog::stdout_color_mt("SERVER");
	ENet enet(logger);
	enet.init();

	Host_Client* client = enet.create_host_client();
	ASSERT_PANIC(client->connect(host, port), "Error connecting client to server");

	while (true) {
		client->poll([](Host_Client* c, Event* event) {
			logger->info("Client Packet received: {}");

			});
	}

	enet.destroy_client(client);

	return EXIT_SUCCESS;
}

int main1() {
	ASSERT_PANIC(enet_initialize() == 0, "error");
	atexit(enet_deinitialize);

	ENetHost* client = enet_host_create(NULL, 1, 2, 0);
	ASSERT_PANIC(client, "error");
	ENetAddress address;
	address.port = port;
	enet_address_set_host(&address, host);
	ENetPeer* peer = enet_host_connect(client, &address, 2);
	ASSERT_PANIC(peer, "error");

	ENetEvent event;
	while (1) {
		while (enet_host_service(client, &event, 0) > 0) {
			switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				spdlog::info("Client connected to server");
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				spdlog::info("Client disconnected from server");
				break;

			case ENET_EVENT_TYPE_RECEIVE:
				spdlog::info("Client Packet received: {}");
				break;
			}
		}
	}
}

