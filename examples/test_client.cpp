#include "server.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>

#include "enet.h"

const char* host = "localhost";
int32_t port = 1234;

int main() {
	auto logger = spdlog::stdout_color_mt("CLIENT");
	ENet enet(logger);

	Host_Client* client = enet.create_host_client();
	ASSERT_PANIC(client->connect(host, port), "Error connecting client to server");

	while (true) {
		client->poll(1000, [](Host_Client* client, Event* event) {
			client->get_logger()->info("Client Packet received: {}");
			});
	}

	enet.destroy_client(client);

	return EXIT_SUCCESS;
}
