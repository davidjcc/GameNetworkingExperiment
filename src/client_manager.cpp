#include "client_manager.h"

client_ptr Game_Client_Manager::add_client(ENetPeer& peer) {
	// TODO(DC): Generate better ids.
	client_id id = m_clients.size();

	m_clients.insert({ id, std::make_shared<Internal_Client>(&peer, m_clients.size(), m_logger) });

	return m_clients[id];
}

