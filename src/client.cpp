#include "client.h"

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

client_ptr Game_Client_Manager::add_client(const ENetPeer& peer) {
	client_ptr result = std::make_shared<Game_Client>(peer, generate_client_name(peer), m_clients.size());
	m_clients.push_back(result);
	return result;
}

void Game_Client_Manager::disconnect_client(const client_id id) {
	if (id < m_clients.size())
		m_clients[id]->disconnect();
}
