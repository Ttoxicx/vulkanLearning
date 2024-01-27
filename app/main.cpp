#include <iostream>
#include "application.h"

int main() {
	std::shared_ptr<FF::Application> app = std::make_shared<FF::Application>();
	try {
		app->run();
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}