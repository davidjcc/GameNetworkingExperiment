#pragma once

#include "state.h"

namespace Net {
	bool IsConnected();
	std::string GetClientName();
	bool InitClient();
	Message PollServer();
	void Shutdown();

	namespace Request {
		void Ping();
		void ListLobbies();
		void CreateLobby(const std::string& name);
		void JoinLobby(const size_t id);
	}
}