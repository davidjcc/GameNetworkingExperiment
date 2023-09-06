#pragma once

#include <spdlog/spdlog.h>

#include "game_messages_generated.h"
#include "config.h"

#include <bs/enet.h>
#include <bs/server.h>

// Base class for the Host_Client and Server to use. This class will
// initialise enet and create the relevant server/client type depending
// on the templated parameter for it. It will also handle the message 
// creation and parsing for the generated game messages. The tick
// function takes in a callback so the server/client and process them
// individually.

template <typename Host_Type>
class Game_Host {
public:
	using tick_cb_t = std::function<void(const Game::Message*, const bs::Packet* packet)>;
	using connect_cb_t = std::function<void()>;

	Game_Host(bs::logger_t logger, const char* host, int32_t port)
		: m_enet(logger)
		, m_host(host)
		, m_port(port)
	{

		// TODO(DC): Make the init/start api the same for both client and server.
		if constexpr (std::is_same_v<Host_Type, bs::Host_Server>) {
			m_host_type = m_enet.create_server(host, port, 32);
			m_host_type->start();
		}
		else if constexpr (std::is_same_v<Host_Type, bs::Host_Client>) {
			m_host_type = m_enet.create_host_client();
			m_host_type->start(host, port);
		}

	}

	~Game_Host() {
		if constexpr (std::is_same_v<Host_Type, bs::Host_Server>) {
			m_enet.destroy_server(m_host_type);
		}
		else if constexpr (std::is_same_v<Host_Type, bs::Host_Client>) {
			m_enet.destroy_client(m_host_type);
		}

	}

	Host_Type* operator->() {
		return m_host_type;
	}

	bs::logger_t get_logger() {
		if (m_host_type)
			return m_host_type->get_logger();

		UNREACHABLE();
		return nullptr;
	}

	void set_tick_callback(tick_cb_t callback) {
		m_tick_callback = callback;
	}

	void set_connect_callback(connect_cb_t callback) {
		m_connect_callback = callback;
	}

	void set_disconnect_callback(connect_cb_t callback) {
		m_disconnect_callback = callback;
	}

	void tick(int32_t timeout = 0) {
		m_host_type->tick(timeout);

		auto& packets = m_host_type->get_packets();

		while (!packets.empty()) {
			auto packet = packets.pop_front();
			switch (packet.get_type()) {
			case bs::Packet::CONNECT:
				if (m_connect_callback)
					m_connect_callback();
				break;
			case bs::Packet::DISCONNECT:
				if (m_disconnect_callback)
					m_disconnect_callback();
				break;

			case bs::Packet::EVENT_RECIEVED:
				if (m_tick_callback) {
					auto packet_bytes = packet.get_bytes();

					flatbuffers::Verifier verifier(packet_bytes.data(), packet_bytes.size());
					if (!Game::VerifyMessageBuffer(verifier)) {
						m_host_type->get_logger()->error("Verifier failed");
						break;
					}

					const auto* message = flatbuffers::GetRoot<Game::Message>(packet_bytes.data());
					m_tick_callback(message, &packet);
				}
				break;

			default:
				m_host_type->get_logger()->error("Unknown packet type: {}", (int)packet.get_type());
			}
		}
	}

	bs::Packet create_client_connect_request() {
		m_host_type->get_logger()->info("Sending client connect request");

		m_builder.Clear();
		auto client_connected = Game::CreateClientConnectedRequest(m_builder);
		auto message = Game::CreateMessage(m_builder, Game::Any_ClientConnectedRequest, client_connected.Union());
		m_builder.Finish(message);
		return std::move(create_packet_from_builder());
	}

	bs::Packet create_client_connect_response(bs::client_id id) {
		m_host_type->get_logger()->info("Sending client connect response");

		m_builder.Clear();
		auto client_connected = Game::CreateClientConnectedResponse(m_builder, id);
		auto message = Game::CreateMessage(m_builder, Game::Any_ClientConnectedResponse, client_connected.Union());
		m_builder.Finish(message);
		return std::move(create_packet_from_builder());
	}

	bs::Packet create_client_disconnect() {
		m_host_type->get_logger()->info("Sending client disconnect request");

		m_builder.Clear();
		auto client_disconnected = Game::CreateClientDisconnected(m_builder);
		auto message = Game::CreateMessage(m_builder, Game::Any_ClientDisconnected, client_disconnected.Union());
		m_builder.Finish(message);
		return std::move(create_packet_from_builder());
	}

	bs::Packet create_client_ready() {
		m_host_type->get_logger()->info("Sending client ready request");

		m_builder.Clear();
		auto client_ready = Game::CreateClientReady(m_builder);
		auto message = Game::CreateMessage(m_builder, Game::Any_ClientReady, client_ready.Union());
		m_builder.Finish(message);
		return std::move(create_packet_from_builder());
	}

	Host_Type* get_host_type() {
		return m_host_type;
	}

private:
	bs::Packet create_packet_from_builder() {
		// TODO(DC): Make the init/start api the same for both client and server.
		if constexpr (std::is_same_v<Host_Type, bs::Host_Server>) {
			return std::move(bs::Packet(NULL, m_builder.GetBufferPointer(), m_builder.GetSize()));
		}
		else if constexpr (std::is_same_v<Host_Type, bs::Host_Client>) {
			return std::move(bs::Packet(m_host_type->get_peer(), m_builder.GetBufferPointer(), m_builder.GetSize()));
		}
	}

	flatbuffers::FlatBufferBuilder m_builder;
	bs::ENet m_enet;

	const char* m_host = nullptr;
	int32_t m_port = -1;

	tick_cb_t m_tick_callback;
	connect_cb_t m_connect_callback;
	connect_cb_t m_disconnect_callback;

	Host_Type* m_host_type = nullptr;
};
