#include "state.h"

#include <fmt/format.h>
#include <enet/enet.h>

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


Message ParseMessage(const char* data, size_t dataLength) {
	std::string dataString(data, dataLength);
	return nlohmann::json::parse(dataString);
}

std::string GenerateClientName(void* peer) {
	// Create a unique identifier for the peer using its IP address and port
	size_t identifier = ((ENetPeer*)peer)->address.host * 2654435761u + ((ENetPeer*)peer)->address.port;

	// Use the identifier to generate an index into the names array
	int index = identifier % ARRAY_COUNT(ANIMAL_NAMES);
	return ANIMAL_NAMES[index];
}

Client CreateClientFromPeer(void* peer) {
	Client client;
	client.status = ClientStatus_Connected;
	client.name = GenerateClientName(peer);

	return client;
}


