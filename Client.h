#pragma once
#include "Misc.h"
#include <string>

struct Client 
{
	Id clientId;
	std::string firstName;
	std::string lastName;
	std::optional<std::string> email;
};

