#include "ClientsDb.h"

ClientsDb::ClientsDb(const std::string& connectionString) : m_connection(connectionString)
{
}

void ClientsDb::ensureDbStructure()
{
	auto w = getWork();
	w.exec("CREATE TABLE IF NOT EXISTS client(\
		client_id SERIAL NOT NULL PRIMARY KEY,\
		first_name VARCHAR(255) NOT NULL,\
		last_name VARCHAR(255) NOT NULL,\
		email VARCHAR(255));");

	w.exec("CREATE TABLE IF NOT EXISTS phone(\
		phone_id SERIAL NOT NULL PRIMARY KEY,\
		client_id INT NOT NULL REFERENCES client(client_id) ON DELETE CASCADE,\
		phone_number VARCHAR(50) NOT NULL)");

	w.commit();
}

void ClientsDb::clearDb()
{
	auto w = getWork();
	w.exec("DELETE FROM client");
	w.commit();
}

Id ClientsDb::addClient(const Client& client)
{
	initializeQueries();
	auto w = getWork();
	auto res = w.exec_prepared("add", client.firstName, client.lastName, client.email);
	auto id = res.cbegin()[0].as<Id>();
	w.commit();
	return id;
}

std::unique_ptr<Client> ClientsDb::getClient(Id clienId)
{
	initializeQueries();
	auto w = getWork();
	auto res = w.exec_prepared("get", clienId);
	if (!res.empty())
	{
		auto field = res.cbegin();
		auto [id, firstName, lastName, email] = field.as<Id, std::string, std::string, std::optional<std::string>>();
		return std::make_unique<Client>(id, firstName, lastName, email);
	}

	return std::unique_ptr<Client>();
}

void ClientsDb::updateClient(const Client& client)
{
	initializeQueries();
	auto w = getWork();
	// при таком подходе обновляются все данные клиента, либо нужно делать отдельные методы на обновление каждого поля
	auto res = w.exec_prepared("update", client.clientId, client.firstName, client.lastName, client.email);
	w.commit();
}

void ClientsDb::removeClient(Id clientId)
{
	initializeQueries();
	auto w = getWork();
	auto res = w.exec_prepared("remove", clientId);
	w.commit();
}

Id ClientsDb::addPhone(const Phone& phone)
{
	initializeQueries();
	auto w = getWork();
	auto res = w.exec_prepared("add_phone", phone.clientId, phone.phoneNumber);
	auto id = res.cbegin()[0].as<Id>();
	w.commit();
	return id;
}

std::unique_ptr<Phone> ClientsDb::getPhone(Id phoneId)
{
	initializeQueries();
	auto w = getWork();
	auto res = w.exec_prepared("get_phone", phoneId);
	if (!res.empty())
	{
		auto field = res.cbegin();
		auto [id, clientId, phoneNumber] = field.as<Id, Id, std::string>();
		return std::make_unique<Phone>(id, clientId, phoneNumber);
	}

	return std::unique_ptr<Phone>();
}

std::list<std::unique_ptr<Phone>> ClientsDb::getPhones(Id clientId)
{
	initializeQueries();
	auto w = getWork();
	auto res = w.exec_prepared("get_phones", clientId);
	std::list<std::unique_ptr<Phone>> resList;
	for (auto field = res.cbegin(); field != res.cend(); ++field)
	{
		auto [id, clientId, phoneNumber] = field.as<Id, Id, std::string>();
		resList.push_back(std::make_unique<Phone>(id, clientId, phoneNumber));
	}

	return resList;
}

void ClientsDb::removePhone(Id phoneId)
{
	initializeQueries();
	auto w = getWork();
	auto res = w.exec_prepared("remove_phone", phoneId);
	w.commit();
}

std::list<std::unique_ptr<Client>> ClientsDb::searchClients(std::string searchTerm)
{
	initializeQueries();
	auto w = getWork();
	auto res = w.exec_prepared("search", searchTerm);
	std::list<std::unique_ptr<Client>> resList;
	for (auto field = res.cbegin(); field != res.cend(); ++field)
	{
		auto [id, firstName, lastName, email] = field.as<Id, std::string, std::string, std::optional<std::string>>();
		resList.push_back(std::make_unique<Client>(id, firstName, lastName, email));
	}

	return resList;
}


void ClientsDb::initializeQueries()
{
	if (queriesInitialized)
	{
		return;
	}

	m_connection.prepare(
		"search",
		"SELECT DISTINCT c.client_id, c.first_name, c.last_name, c.email\
		FROM client c\
		LEFT JOIN phone p ON p.client_id = c.client_id\
		WHERE c.first_name LIKE '%'|| $1 || '%'\
		OR c.last_name LIKE '%'|| $1 || '%'\
		OR c.email LIKE '%'|| $1 || '%'\
		OR p.phone_number LIKE '%'|| $1 || '%'");

	m_connection.prepare("add", "INSERT INTO client(first_name, last_name, email)VALUES($1, $2, $3) RETURNING client_id");

	m_connection.prepare("get", "SELECT client_id, first_name, last_name, email FROM client WHERE client_id=$1");

	m_connection.prepare("remove", "DELETE FROM client WHERE client_id=$1");

	m_connection.prepare("update", "UPDATE client SET first_name = $2, last_name = $3, email = $4 WHERE client_id = $1");

	m_connection.prepare("add_phone", "INSERT INTO phone(client_id, phone_number)VALUES($1, $2) RETURNING phone_id");

	m_connection.prepare("get_phone", "SELECT phone_id, client_id, phone_number FROM phone WHERE phone_id=$1");

	m_connection.prepare("get_phones", "SELECT phone_id, client_id, phone_number FROM phone WHERE client_id=$1 ORDER BY phone_id");

	m_connection.prepare("remove_phone", "DELETE FROM phone WHERE phone_id=$1");

	queriesInitialized = true;
}

pqxx::work ClientsDb::getWork()
{
	return pqxx::work(m_connection);
}
