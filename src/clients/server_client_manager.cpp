#include "clients/server_client_manager.h"
#include "clients/server_client.h"

server_client_ptr Server_Client_Manager::add_client(ENetPeer& peer) {
	// TODO(DC): Generate better ids.
	client_id id = m_clients.size();

	m_clients.insert({ id, std::make_shared<Server_Client>(&peer, m_clients.size(), m_logger) });

	return m_clients[id];
}

