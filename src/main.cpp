#include <Platform/Linux/SDLApplication.hpp>
#include <Debug/Logger.hpp>
#include <print>

int main(int argc, char* argv[]) {
	Izo::Logger::instance().info("Izotrox is booting...");
	SDLApplication app("Izotrox Demo", 800, 600);
    app.run();
	return 0;
}
