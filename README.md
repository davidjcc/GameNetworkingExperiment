# GameNetworkingTest
Experimenting with games networking programming.

## BS
A simple wrapper of the ENet library. The wrapper is in the `bs` folder.

## Flatbuffers
The samples use the `flatbuffers` library to serialize data. There is a pre-build step that will convert the `fbs` files in to C++ headers.

## Libraries:
- [CPM](https://github.com/cpm-cmake/CPM.cmake) - Cmake dependencies management
- [ENet](http://enet.bespin.org/index.html) - UDP networking library
- [fmtlib](https://github.com/fmtlib/fmt) - String formatting
- [spdlog](https://github.com/gabime/spdlog) - Server logging


## Example Server
```cpp
#include <bs/enet.h>

#include <spdlog/sinks/stdout_color_sinks.h>

int main() {
	// Create the main ENet object.
	auto logger = spdlog::stdout_color_mt("SERVER");
	bs::ENet enet(logger);

	// Create the server.
	bs::Host_Server* server = enet.create_server("127.0.0.1", 1234, 1);

	// Start the server running.
	server->start();

	while (1) {
		// Update the server's packets (if there are any).
		server->tick(0);

		// Process the packet's.
		auto& packets = server->get_packets();
		while (!packets.empty()) {
			auto packet = packets.pop_front();
			switch (packet.get_type()) {
			case bs::Packet::NONE: break;

			case bs::Packet::CONNECT: {
				server->get_logger()->info("Client connected");
				break;
			}

			case bs::Packet::DISCONNECT: {
				server->get_logger()->info("Client disconnected");
				break;
			}

			case bs::Packet::EVENT_RECIEVED: {
				server->get_logger()->info("Packet recieved");
				break;
			}
			}
		}
	}

	return 0;
}
```

## Example Client
```cpp
#include <bs/enet.h>

#include <spdlog/sinks/stdout_color_sinks.h>

int main() {
	// Create the main enet object.
	auto logger = spdlog::stdout_color_mt("CLIENT");
	bs::ENet enet(logger);

	// Create the client.
	bs::Host_Client* client = enet.create_host_client();

	// Start the client running and connect to the server.
	client->start("127.0.0.1", 1234);

	while (1) {
		// Update the client's packets (if there are any).
		client->tick(0);

		// Process the packets.
		auto& packets = client->get_packets();
		while (!packets.empty()) {
			auto packet = packets.pop_front();
			switch (packet.get_type()) {
			case bs::Packet::NONE: break;

			case bs::Packet::CONNECT: {
				client->get_logger()->info("Connected to server");
				break;
			}

			case bs::Packet::DISCONNECT: {
				client->get_logger()->info("Disconnected from serve");
				break;
			}

			case bs::Packet::EVENT_RECIEVED: {
				client->get_logger()->info("Packet recieved");
				break;
			}
			}
		}
	}

	return 0;
}
```
