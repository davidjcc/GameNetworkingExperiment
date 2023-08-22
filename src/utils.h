#pragma once

#include <fmt/format.h>
#include <cassert>

#define ASSERT_PANIC(COND, FMT, ...) \
	if (!(COND)) { fmt::print("ERROR: {}:{} ", __FUNCTION__, __LINE__); fmt::print(FMT, __VA_ARGS__); fmt::print("\n"); exit(EXIT_FAILURE); }
#define PANIC(MSG, ...) ASSERT_PANIC(false, MSG, __VA_ARGS__)
#define UNREACHABLE() ASSERT_PANIC(false, "Unreachable code reached at {}:{}\n", __FILE__, __LINE__)

#define NO_COPY_NO_MOVE(CLASS) \
	CLASS(const CLASS&) = delete; \
	CLASS& operator=(const CLASS&) = delete; \
	CLASS(CLASS&&) = delete; \
	CLASS& operator=(CLASS&&) = delete;

#define SAFE_DELETE(PTR) if ((PTR != nullptr)) { delete PTR; PTR = nullptr; }