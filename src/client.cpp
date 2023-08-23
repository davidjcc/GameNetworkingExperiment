#include "client.h"
#include "enet/enet.h"

static const char* ANIMAL_NAMES[] = {
	"Elephant", "Dolphin", "Lion", "Tiger", "Bear", "Wolf", "Giraffe", "Kangaroo", "Penguin", "Whale",
	"Jellyfish", "Fox", "Monkey", "Koala", "Gazelle", "Hippopotamus", "Otter", "Crocodile", "Porcupine", "Horse",
	"Frog", "Rhinoceros", "Cheetah", "Walrus", "Squirrel", "Sloth", "Bison", "Gorilla", "Badger", "Skunk",
	"Zebra", "Ostrich", "Llama", "Anteater", "Moose", "Beaver", "Chimpanzee", "Panda", "Aardvark", "Rabbit",
	"Snail", "Camel", "Meerkat", "Hamster", "Bat", "Eagle", "Shark", "Octopus", "Toucan", "Peacock",
	"Flamingo", "Goat", "Sheep", "Deer", "Hedgehog", "Wildebeest", "Buffalo", "Antelope", "Orangutan", "Lynx",
	"Raccoon", "Tapir", "Armadillo", "Mole", "Ferret", "Platypus", "Seal", "Owl", "Turtle", "Lemur",
	"Swan", "Snake", "Parrot", "Polar Bear", "Starfish", "Sea Lion", "Pufferfish", "Hyena", "Crab", "Gecko",
	"Salmon", "Spider", "Swordfish", "Ray", "Catfish", "Albatross", "Seahorse", "Piranha", "Coyote", "Parakeet",
	"Iguana", "Hummingbird", "Barracuda", "Falcon", "Manta Ray", "Mongoose", "Duck"
};

static std::string generate_client_name(const ENetPeer& peer) {
	// Create a unique identifier for the peer using its IP address and port
	size_t identifier = (peer.address.host * 2654435761u + (peer.address.port));

	// Use the identifier to generate an index into the names array
	size_t index = identifier % std::size(ANIMAL_NAMES);
	return ANIMAL_NAMES[index];
}

Game_Client::Game_Client(const std::string& name, const client_id slot)
	: m_name(name), m_slot(slot) {}

client_ptr Game_Client_Manager::add_client(ENetPeer& peer) {
	const auto client_name = generate_client_name(peer);

	auto result = std::make_shared<Internal_Client>(&peer, m_clients.size());
	m_clients.push_back(result);
	return m_clients[m_clients.size() - 1];
}

Internal_Client::Internal_Client(ENetPeer* peer, client_id slot)
	: m_peer(peer), Game_Client(generate_client_name(*peer), slot)
{
}

Host_Client::Host_Client()
	: Game_Client("", -1) 
{
	m_client = enet_host_create(NULL, 1, 2, 0);

	if (!m_client) {
		PANIC("An error occurred while trying to create an Host client");
	}
}

Host_Client::~Host_Client() {
	enet_host_destroy(m_client);
}

bool Host_Client::connect(const char* host, int32_t port) {
	m_host = host;
	m_port = port;

	ENetAddress address;
	enet_address_set_host(&address, m_host);
	address.port = m_port;
	m_server = enet_host_connect(m_client, &address, 2);
	if (!m_server) {
		PANIC("An error occurred while trying to create an Host peer");
		return false;
	}

	return true;
}

void Host_Client::on_connect(Event& event) {
	auto data = event.get_string();
	int a = 0;
}

void Host_Client::on_disconnect(Event& event) {
}

void Host_Client::poll(poll_callback cb) {
	ENetEvent enet_event{};
	while (enet_host_service(m_client, &enet_event, 1000) > 0) {
		Event event(enet_event);

		switch (event.get_type()) {
		case Event::CONNECT: {
			cb(this, &event);
			on_connect(event);
		} break;

		case Event::DISCONNECT: {
			cb(this, &event);
			on_disconnect(event);
		} break;

		case Event::EVENT_RECEIVED: {
			cb(this, &event);
		} break;
		}
	}
}

client_ptr Game_Client_Manager::get_client_from_event(Event& event) {
	const auto identifier = generate_client_name(*event.get_peer());

	for (auto& client : m_clients) {
		if (client->get_name() == identifier) {
			return client;
		}
	}

	return nullptr;
}
