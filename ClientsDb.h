#pragma once
#include <string>
#include <list>
#include <memory>
#include <pqxx/pqxx>
#include "Client.h"
#include "Phone.h"



class ClientsDb final
{
public:
	
	ClientsDb(const std::string& connectionString);

	void ensureDbStructure();

	void clearDb();

	Id addClient(const Client& client);

	std::unique_ptr<Client> getClient(Id clienId);

	void updateClient(const Client& client);

	void removeClient(Id clientId);

	Id addPhone(const Phone& phone);

	std::unique_ptr<Phone> getPhone(Id phoneId);

	std::list<std::unique_ptr<Phone>> getPhones(Id clientId);

	void removePhone(Id phoneId);

	std::list<std::unique_ptr<Client>> searchClients(std::string searchTerm);

private:
	pqxx::connection m_connection;
	bool queriesInitialized = false;
	void initializeQueries();
	pqxx::work getWork();
};
