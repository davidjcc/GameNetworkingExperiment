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

void to_json(nlohmann::json& nlohmann_json_j, const Lobby& nlohmann_json_t) {
	nlohmann_json_j["name"] = nlohmann_json_t.name; 
	nlohmann_json_j["id"] = nlohmann_json_t.id; 

	for (size_t i = 0; i < nlohmann_json_t.clients.size(); ++i) {
		std::string key = fmt::format("client_{}", i);
		if (nlohmann_json_t.clients[i]) {
			nlohmann_json_j["clients"][key] = nlohmann_json_t.clients[i]->name;
		}
		else {
			nlohmann_json_j["clients"][key] = "empty";
		}
	}
} 
inline void from_json(const nlohmann::json& nlohmann_json_j, Lobby& nlohmann_json_t) {
	Lobby nlohmann_json_default_obj; 
	nlohmann_json_t.name = nlohmann_json_j.value("name", nlohmann_json_default_obj.name); 
	nlohmann_json_t.id = nlohmann_json_j.value("id", nlohmann_json_default_obj.id); 
	//nlohmann_json_t.clients = nlohmann_json_j.value("clients", nlohmann_json_default_obj.clients);
};


