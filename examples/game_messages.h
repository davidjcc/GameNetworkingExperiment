#pragma once
#include <enet/enet.h>

#include "game_messages_generated.h"

#include "packet.h"

namespace Game::Messages {
	inline Packet create_client_connect_message(ENetPeer* peer, flatbuffers::FlatBufferBuilder& builder) {
		builder.Clear();

		auto client_connected = CreateClientConnectedMessage(builder);
		auto message = CreateMessage(builder, AnyMessage_ClientConnectedMessage, client_connected.Union());
		builder.Finish(message);

		return Packet(peer, builder.GetBufferPointer(), builder.GetSize());
	};

	inline Packet create_client_disconnect_message(ENetPeer* peer, flatbuffers::FlatBufferBuilder& builder) {
		builder.Clear();

		auto client_disconnected = CreateClientDisconnectedMessage(builder);
		auto message = CreateMessage(builder, AnyMessage_ClientDisconnectedMessage, client_disconnected.Union());
		builder.Finish(message);

		return Packet(peer, builder.GetBufferPointer(), builder.GetSize());
	};

	inline Packet create_client_ready_message(ENetPeer* peer, flatbuffers::FlatBufferBuilder& builder) {
		builder.Clear();

		auto client_ready = CreateClientReadyMessage(builder);
		auto message = CreateMessage(builder, AnyMessage_ClientReadyMessage, client_ready.Union());
		builder.Finish(message);

		return Packet(peer, builder.GetBufferPointer(), builder.GetSize());
	};

	inline Packet create_player_moved_message(ENetPeer* peer, flatbuffers::FlatBufferBuilder& builder, float x, float y) {
		builder.Clear();

		auto player_moved = CreatePlayerMovedMessage(builder, Game::CreateVec2(builder, x, y));
		auto message = CreateMessage(builder, AnyMessage_PlayerMovedMessage, player_moved.Union());
		builder.Finish(message);

		return Packet(peer, builder.GetBufferPointer(), builder.GetSize());
	};

	inline Packet create_ball_moved_message(ENetPeer* peer, flatbuffers::FlatBufferBuilder& builder, float x, float y) {
		builder.Clear();

		auto ball_moved = CreateBallMovedMessage(builder, Game::CreateVec2(builder, x, y));
		auto message = CreateMessage(builder, AnyMessage_BallMovedMessage, ball_moved.Union());
		builder.Finish(message);

		return Packet(peer, builder.GetBufferPointer(), builder.GetSize());
	};
}

