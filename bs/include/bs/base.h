#pragma once
#include "utils.h"

#include <spdlog/spdlog.h>

namespace bs {
	using client_id = int32_t;

	class Base_Client {
	public:
		NO_COPY_NO_MOVE(Base_Client);

		enum State {
			NONE = 0,
			CONNECTED,
			DISCONNECTED,
		};

		Base_Client(const client_id id, logger_t& logger)
			: m_logger(logger), m_id(id) {}

		const client_id get_id() const { return m_id; }
		void set_id(const client_id id) { m_id = id; }

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
		client_id m_id = -1;
		logger_t m_logger;
	};
}
