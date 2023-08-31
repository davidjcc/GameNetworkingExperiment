#pragma once
#include <enet/enet.h>

#include "game_messages_generated.h"

#include "packet.h"

namespace Game::Messages {
	inline Packet create_client_connect_request(ENetPeer* peer, flatbuffers::FlatBufferBuilder& builder) {
		builder.Clear();

		auto client_connected = CreateClientConnectedRequest(builder);
		auto message = CreateMessage(builder, Any_ClientConnectedRequest, client_connected.Union());
		builder.Finish(message);

		return Packet(peer, builder.GetBufferPointer(), builder.GetSize());
	}

	inline Packet create_client_connected_response(ENetPeer* peer, flatbuffers::FlatBufferBuilder& builder, client_id id) {
		builder.Clear();

		auto client_connected = CreateClientConnectedResponse(builder, id);
		auto message = CreateMessage(builder, Any_ClientConnectedResponse, client_connected.Union());
		builder.Finish(message);

		return Packet(peer, builder.GetBufferPointer(), builder.GetSize());
	}


	inline Packet create_client_connect_response(ENetPeer* peer, flatbuffers::FlatBufferBuilder& builder, size_t client_id) {
		builder.Clear();

		auto client_connected = CreateClientConnectedResponse(builder);
		auto message = CreateMessage(builder, Any_ClientConnectedResponse, client_connected.Union());
		builder.Finish(message);

		return Packet(peer, builder.GetBufferPointer(), builder.GetSize());
	}


	inline Packet create_client_disconnect(ENetPeer* peer, flatbuffers::FlatBufferBuilder& builder) {
		builder.Clear();

		auto client_disconnected = CreateClientDisconnected(builder);
		auto message = CreateMessage(builder, Any_ClientDisconnected, client_disconnected.Union());
		builder.Finish(message);

		return Packet(peer, builder.GetBufferPointer(), builder.GetSize());
	}

	inline Packet create_client_ready(ENetPeer* peer, flatbuffers::FlatBufferBuilder& builder) {
		builder.Clear();

		auto client_ready = CreateClientReady(builder);
		auto message = CreateMessage(builder, Any_ClientReady, client_ready.Union());
		builder.Finish(message);

		return Packet(peer, builder.GetBufferPointer(), builder.GetSize());
	}

	inline Packet create_player_moved(ENetPeer* peer, flatbuffers::FlatBufferBuilder& builder, float x, float y) {
		builder.Clear();

		auto player_moved = CreatePlayerMoved(builder, Game::CreateVec2(builder, x, y));
		auto message = CreateMessage(builder, Any_PlayerMoved, player_moved.Union());
		builder.Finish(message);

		return Packet(peer, builder.GetBufferPointer(), builder.GetSize());
	}

	inline Packet create_ball_moved(ENetPeer* peer, flatbuffers::FlatBufferBuilder& builder, float x, float y) {
		builder.Clear();

		auto ball_moved = CreateBallMoved(builder, Game::CreateVec2(builder, x, y));
		auto message = CreateMessage(builder, Any_BallMoved, ball_moved.Union());
		builder.Finish(message);

		return Packet(peer, builder.GetBufferPointer(), builder.GetSize());
	}

}

