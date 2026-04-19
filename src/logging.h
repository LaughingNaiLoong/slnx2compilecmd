#pragma once

#include <format>
#include <iostream>

#ifdef _DEBUG
#define LOG_DEBUG(...)                                                         \
  std::cout << "[DEBUG] " << std::format(__VA_ARGS__) << std::endl
#else
#define LOG_DEBUG(...)
#endif

#define LOG_INFO(...)                                                          \
  std::cout << "[INFO] " << std::format(__VA_ARGS__) << std::endl

#define LOG_WARN(...)                                                          \
  std::cout << "[WARN] " << std::format(__VA_ARGS__) << std::endl