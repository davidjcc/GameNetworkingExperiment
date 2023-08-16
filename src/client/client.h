#pragma once

#include "state.h"

namespace Net {
	bool IsConnected();
	std::string GetClientName();
	bool InitClient();

	using PollCallback = void(*)(const Packet& packet);
	void PollServer(PollCallback cb);
	void Shutdown();

	namespace Request {
		void Ping(const uint32_t timestamp);
		void CreateLobby(const std::string& name);
		void JoinLobby(const size_t id);
	}
}