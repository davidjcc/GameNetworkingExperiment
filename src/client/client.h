#pragma once

#include "state.h"

namespace Net {
	bool IsConnected();
	std::string GetClientName();
	bool InitClient();

	using PollCallback = void(*)(const Packet& packet);
	void PollServer(PollCallback cb);
	void Shutdown();

	int32_t GetLobbyID();
	bool IsInLobby();

	namespace Request {
		void Ping(const uint32_t timestamp);
		void CreateLobby(const std::string& name);
		void JoinLobby(const lobby_id id);
		void UpdateLobby();
		void GetState();
		void PlayerMoved(const float x, const float y);
		void SetReady(const bool ready);
	}
}