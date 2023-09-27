#include <cstdint>
#include <memory>
#include <spdlog/common.h>
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

class Log {
  public:
	static void Init(spdlog::level::level_enum logLevel = spdlog::level::trace);

	inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }

  private:
	static std::shared_ptr<spdlog::logger> s_Logger;
};

#ifndef SUNSET_DIST
	#define LOG_TRACE(...) ::Log::GetLogger()->trace(__VA_ARGS__)
	#define LOG_INFO(...) ::Log::GetLogger()->info(__VA_ARGS__)
	#define LOG_WARN(...) ::Log::GetLogger()->warn(__VA_ARGS__)
	#define LOG_ERROR(...) ::Log::GetLogger()->error(__VA_ARGS__)
	#define LOG_CRITICAL(...) ::Log::GetLogger()->critical(__VA_ARGS__)
#else
	#define LOG_TRACE(...)
	#define LOG_INFO(...)
	#define LOG_WARN(...)
	#define LOG_ERROR(...)
	#define LOG_CRITICAL(...)
#endif // !SUNSET_DIST

#ifdef SUNSET_DEBUG
static const bool enableValidationLayers = true;
static const char* requiredValidationLayers[] = {"VK_LAYER_KHRONOS_validation"};
static const uint32_t requiredValidationLayersSize = 1;
#else
static const bool enableValidationLayers = false;
static const char* requiredValidationLayers[] = {};
static const uint32_t requiredValidationLayersSize = 0;
#endif
