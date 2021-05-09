#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <deque>
#include <vector>
#include <iostream>

#ifdef _WIN32
#define _WIN32_WINNT 0X0A00
#endif

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include <spdlog/spdlog.h>
