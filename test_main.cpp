#include <iostream>
#include <catch2/catch_session.hpp>

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "ru_RU.UTF-8");
	return Catch::Session().run(argc, argv);
}