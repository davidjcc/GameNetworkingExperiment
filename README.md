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
	auto logger = spdlog::stdout_color_mt("SERVER");

	ENet enet(logger);

	Host_Server* server = enet.create_server("127.0.0.1", 1234, 1);
	server->start();

	while (1) {
		server->tick(0);

		auto& packets = server->get_packets();
		while (!packets.empty()) {
			auto packet = packets.pop_front();
			switch (packet.get_type()) {
			case Packet::NONE: break;

			case Packet::CONNECT: {
				server->get_logger()->info("Client connected");
				break;
			}

			case Packet::DISCONNECT: {
				server->get_logger()->info("Client disconnected");
				break;
			}

			case Packet::EVENT_RECIEVED: {
				server->get_logger()->info("Packet recieved");
				break;
			}
			}
		}
	}

	enet.destroy_server(server);
	return 0;
}

```

## Example Client
```cpp
#include <bs/enet.h>

#include <spdlog/sinks/stdout_color_sinks.h>

int main() {
	auto logger = spdlog::stdout_color_mt("CLIENT");

	ENet enet(logger);

	Host_Client* client = enet.create_host_client();
	client->start("127.0.0.1", 1234);

	while (1) {
		client->tick(0);

		auto& packets = client->get_packets();
		while (!packets.empty()) {
			auto packet = packets.pop_front();
			switch (packet.get_type()) {
			case Packet::NONE: break;

			case Packet::CONNECT: {
				client->get_logger()->info("Connected to server");
				break;
			}

			case Packet::DISCONNECT: {
				client->get_logger()->info("Disconnected from serve");
				break;
			}

			case Packet::EVENT_RECIEVED: {
				client->get_logger()->info("Packet recieved");
				break;
			}
			}
		}
	}

	enet.destroy_client(client);
	return 0;
}
```
