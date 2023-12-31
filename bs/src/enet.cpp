#include "bs/enet.h"

#include <enet/enet.h>

namespace bs {
	ENet::ENet(logger_t& logger) : m_logger(logger)
	{
		m_logger->info("Initialising ENet");

		ASSERT_PANIC(m_state == STATE_UNINITIALISED, "Trying to initialise the ENet object when it is already initialised.");

		if (enet_initialize() != 0) {
			PANIC("An error occurred while initializing ENet.");
		}

		m_logger->info("ENet initialised");
		m_state = STATE_INITIALISED;
	}

	ENet::~ENet() {
		if (m_server) {
			destroy_server(m_server);
		}

		for (auto& client : m_clients) {
			destroy_client(client);
		}

		if (m_state == STATE_INITIALISED) {
			enet_deinitialize();
		}
	}

	Host_Server* ENet::create_server(const char* host, int port, int max_clients) {
		ASSERT_PANIC(m_server == nullptr, "Trying to create a new server when one is already created.");

		m_logger->info("Creating new server: host => {} port => {} max_clients => {}", host, port, max_clients);

		auto* result = new Host_Server(host, port, max_clients, m_logger);

		if (result == nullptr) {
			PANIC("Failed trying to create a new server");
		}

		m_server = result;

		return result;
	}

	void ENet::destroy_server(Host_Server* server) {
		m_logger->info("Destroying server");
		SAFE_DELETE(server);
	}

	Host_Client* ENet::create_host_client() {
		auto* result = new Host_Client(m_logger);

		m_clients.push_back(result);

		if (result == nullptr) {
			PANIC("Failed trying to create a new client");
		}

		return result;
	}

	void ENet::destroy_client(Host_Client* client) {
		m_logger->info("Destroying client");
		if (client) {
			client->disconnect();
			SAFE_DELETE(client);
		}
	}
}
