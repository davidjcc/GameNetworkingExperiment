#pragma once
#include "utils.h"

#include <spdlog/spdlog.h>
#include <enet/enet.h>

using client_id = size_t;

class Base_Client {
public:
	NO_COPY_NO_MOVE(Base_Client);

	enum State {
		NONE = 0,
		CONNECTED,
		DISCONNECTED,
	};

	Base_Client(const client_id slot, logger_t& logger)
		: m_logger(logger), m_slot(slot) {}

	const size_t get_slot() const { return m_slot; }
	const State& get_state() const { return m_state; }
	logger_t& get_logger() { return m_logger; }

	void connect() {
		m_state = CONNECTED;
	}

	void disconnect() {
		m_state = DISCONNECTED;
	}

protected:
	State m_state = NONE;
	client_id m_slot;
	logger_t m_logger;
};
