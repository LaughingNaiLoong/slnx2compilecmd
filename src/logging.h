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
  std::cout << "\033[0m" << std::format(__VA_ARGS__) << "\033[0m" << std::endl

#define LOG_WARN(...)                                                          \
  std::cout << "\033[93m" << std::format(__VA_ARGS__) << "\033[0m" << std::endl

#define LOG_ERROR(...)                                                         \
  std::cout << "\033[91m" << std::format(__VA_ARGS__) << "\033[0m" << std::endl