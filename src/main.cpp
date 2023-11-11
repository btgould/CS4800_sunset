#include <stdexcept>

#include "util/log.hpp"
#include "util/profiler.hpp"
#include "application/application.hpp"

int main() {
	Log::Init(spdlog::level::info);
	Application app;

	try {
		PROFILE_BEGIN_SESSION("Runtime", "Runtime_Profile.json");
		app.run();
		PROFILE_END_SESSION();
	} catch (const std::exception& e) {
		LOG_ERROR("{0}", e.what());
		PROFILE_END_SESSION();
		return EXIT_FAILURE;
	}

	app.shutdown();

	return EXIT_SUCCESS;
}
