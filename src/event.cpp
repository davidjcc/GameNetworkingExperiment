#include "event.h"
#include "utils.h"

Event::~Event() {
}

std::string Event::get_string() const {
	std::string result = "";

	if (!m_bytes.empty()) {
		result.assign(reinterpret_cast<const char*>(m_bytes.data()), m_bytes.size());
	}

	return result;
}

std::vector<uint8_t> Event::get_bytes() const {
	return m_bytes;
}
