#include <catch2/catch_test_macros.hpp>
#include "ClientsDb.h"

static const std::string cStr("host=localhost port=5432 dbname=clients user=postgres password=admin");

TEST_CASE("Содание БД", "[ClientsDb]")
{
	ClientsDb db(cStr);
	db.ensureDbStructure();
	db.clearDb();
}

TEST_CASE("Добавление клиента", "[ClientsDb]")
{
	SECTION("Все данные")
	{
		ClientsDb db(cStr);
		Client c = { .firstName = "Антон", .lastName = "Брыляков", .email = "yoll@rambler.ru" };
		auto id = db.addClient(c);

		auto client = db.getClient(id);
		REQUIRE(client != nullptr);
		CHECK(client->firstName == "Антон");
		CHECK(client->lastName == "Брыляков");
		CHECK(client->email == "yoll@rambler.ru");
	}

	SECTION("Без email")
	{
		ClientsDb db(cStr);
		Client c = { .firstName = "Иван", .lastName = "Иванов" };
		auto id = db.addClient(c);

		auto client = db.getClient(id);
		REQUIRE(client != nullptr);
		CHECK(client->firstName == "Иван");
		CHECK(client->lastName == "Иванов");
		CHECK(!client->email.has_value());
	}
}

TEST_CASE("Удаление клиента", "[ClientsDb]")
{
	ClientsDb db(cStr);
	db.ensureDbStructure();
	db.clearDb();

	Client c = { .firstName = "Антон", .lastName = "Брыляков", .email = "yoll@rambler.ru" };
	auto id = db.addClient(c);

	auto client = db.getClient(id);
	REQUIRE(client != nullptr);
	db.removeClient(client->clientId);
	auto removedClient = db.getClient(id);
	CHECK(removedClient == nullptr);
}

TEST_CASE("Обновление данных клиента", "[ClientsDb]")
{
	ClientsDb db(cStr);
	db.ensureDbStructure();
	db.clearDb();

	Client c = { .firstName = "Антон", .lastName = "Брыляков", .email = "yoll@rambler.ru" };
	auto id = db.addClient(c);

	auto client = db.getClient(id);
	REQUIRE(client != nullptr);
	
	{
		Client c = { .clientId = id, .firstName = "Иван", .lastName = "Иванов", .email = "ii@rambler.ru" };
		db.updateClient(c);
		client = db.getClient(id);
		CHECK(client->firstName == "Иван");
		CHECK(client->lastName == "Иванов");
		CHECK(client->email == "ii@rambler.ru");
	}
}

TEST_CASE("Номера телефонов", "[ClientsDb]")
{
	SECTION("Два номера")
	{
		ClientsDb db(cStr);
		Client c = { .firstName = "Антон", .lastName = "Брыляков", .email = "yoll@rambler.ru" };
		auto id = db.addClient(c);

		{
			Phone p = { .clientId = id, .phoneNumber = "89612223333" };
			auto phoneId = db.addPhone(p);
			auto phone = db.getPhone(phoneId);
			REQUIRE(phone != nullptr);
			CHECK(phone->phoneNumber == "89612223333");
		}

		{
			Phone p = { .clientId = id, .phoneNumber = "89611112224" };
			auto phoneId = db.addPhone(p);
			auto phone = db.getPhone(phoneId);
			REQUIRE(phone != nullptr);
			CHECK(phone->phoneNumber == "89611112224");
		}

		{
			auto phones = db.getPhones(id);
			REQUIRE(phones.size() == 2);
			auto it = phones.cbegin();
			auto phone1 = it->get();
			CHECK(phone1->phoneNumber == "89612223333");
			auto phone2 = (++it)->get();
			CHECK(phone2->phoneNumber == "89611112224");
		}
	}

	SECTION("Удаление номер")
	{
		ClientsDb db(cStr);
		Client c = { .firstName = "Антон", .lastName = "Брыляков", .email = "yoll@rambler.ru" };
		auto id = db.addClient(c);

		
		Phone p = { .clientId = id, .phoneNumber = "89612223333" };
		auto phoneId = db.addPhone(p);
		auto phone = db.getPhone(phoneId);
		REQUIRE(phone != nullptr);
		db.removePhone(phoneId);
		phone = db.getPhone(phoneId);
		CHECK(phone == nullptr);
		auto phones = db.getPhones(id);
		CHECK(phones.size() == 0);
	}
}

TEST_CASE("Поиск клиента", "[ClientsDb]")
{
	ClientsDb db(cStr);
	db.clearDb();

	{
		Client c = { .firstName = "Антон", .lastName = "Брыляков", .email = "yoll@rambler.ru" };
		auto id = db.addClient(c);
		Phone p = { .clientId = id, .phoneNumber = "89612223333" };
		auto phoneId = db.addPhone(p);
	}

	{
		Client c = { .firstName = "Иван", .lastName = "Иванов" };
		auto id = db.addClient(c);
		Phone p = { .clientId = id, .phoneNumber = "89611112222" };
		auto phoneId = db.addPhone(p);
	}

	SECTION("Успешный поиск по имени")
	{
		auto clients = db.searchClients("Антон");
		REQUIRE(clients.size() == 1);
		auto client = (clients.cbegin())->get();
		CHECK(client->lastName == "Брыляков");
	}

	SECTION("Успешный поиск по фамилии")
	{
		auto clients = db.searchClients("Иванов");
		REQUIRE(clients.size() == 1);
		auto client = (clients.cbegin())->get();
		CHECK(client->lastName == "Иванов");
	}

	SECTION("Успешный поиск по email")
	{
		auto clients = db.searchClients("yoll@rambler.ru");
		REQUIRE(clients.size() == 1);
		auto client = (clients.cbegin())->get();
		CHECK(client->lastName == "Брыляков");
	}

	SECTION("Успешный поиск по номеру телефона")
	{
		auto clients = db.searchClients("89612223333");
		REQUIRE(clients.size() == 1);
		auto client = (clients.cbegin())->get();
		CHECK(client->lastName == "Брыляков");
	}

	SECTION("Неуспешный поиск")
	{
		auto clients = db.searchClients("Петров");
		REQUIRE(clients.size() == 0);
	}
}