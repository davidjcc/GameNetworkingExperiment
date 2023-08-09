#pragma once

#include "state.h"

namespace Net {
	bool IsConnected();
	bool InitClient();
	Message Poll();
	void Ping();
	void Shutdown();
}