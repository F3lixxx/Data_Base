#include <iostream>
#include <pqxx/pqxx>
#include "Windows.h"

void create_table(pqxx::connection &con){
	pqxx::work transaction(con);
	
	transaction.exec(
		"CREATE TABLE IF NOT EXISTS client_info ("
		"id INTEGER PRIMARY KEY,"
		"first_name TEXT,"
		"last_name TEXT,"
		"email VARCHAR)");
	transaction.exec(
		"CREATE TABLE IF NOT EXISTS phone_number("
		"client_id INTEGER ,"
		"phone TEXT PRIMARY KEY, "
	    "FOREIGN KEY (client_id) REFERENCES client_info(id) ON DELETE CASCADE)");
	transaction.commit();
}

void add_client(pqxx::connection& con) {
	pqxx::work trans(con);

	// Подготовка запроса на добавление клиента
	con.prepare("add_client", "INSERT INTO client_info(id, first_name, last_name, email) VALUES ($1, $2, $3, $4) RETURNING id");

	// Подготовленное выражение для вставки номера телефона
	con.prepare("insert_phone", "INSERT INTO phone_number(client_id, phone) VALUES($1, $2)");

	std::cout << "Enter Id: ";
	int id_cl;
	std::cin >> id_cl;

	std::cout << "Enter First Name: ";
	std::string first_name;
	std::cin >> first_name;

	std::cout << "Enter Last Name: ";
	std::string last_name;
	std::cin >> last_name;

	std::cout << "Enter Email: ";
	std::string email;
	std::cin >> email;

	// Запрос номеров телефонов для данного клиента
		pqxx::result res = trans.exec_prepared("add_client", id_cl, first_name, last_name, email);
		int client_id = res[0]["id"].as<int>();

		std::string phone_number;
		do {
			std::cout << "Enter some Phone Number (or 'done' to finish): ";
			std::cin >> phone_number;

			if (phone_number != "done") {
				try {
					trans.exec_prepared("insert_phone", client_id, phone_number);
				}
				catch (const std::exception& e) {
					std::cerr << "Error adding phone number: " << e.what() << std::endl;
					break;
				}
			}
		} while (phone_number != "done");

		trans.commit();
	}

void update_info(pqxx::connection& con) {
	pqxx::work transaction(con);
	std::string new_value;
	int num;
	int client_id;

	std::cout << "Enter what you want do!" << std::endl;
		std::cout << "1-Update First Name!" << std::endl;
		std::cout << "2-Update Last Name!" << std::endl;
		std::cout << "3-Update Email!" << std::endl;
		std::cout << "4-Update Phone number!" << std::endl;
		std::cin >> num;

	std::cout << "Write new value: ";
	std::cin >> new_value;

	std::string column_name;
	switch (num) {
	case 1:
		column_name = "first_name";
		break;
	case 2:
		column_name = "last_name";
		break;
	case 3:
		column_name = "email";
		break;
	case 4:
		column_name = "phone_number";
		break;
	default:
		std::cerr << "Wrong variant" << std::endl;
		return;
	}

	std::cout << "Enter client ID: ";
	std::cin >> client_id;

	con.prepare("update_info", "UPDATE client_info SET " + column_name + " = $1 WHERE id = $2");

	transaction.exec_prepared("update_info", new_value, client_id); 
	transaction.commit();
}

void delete_info(pqxx::connection& con) {
    pqxx::work transaction(con);
    int num;
    int client_id;

    std::cout << "Enter what you want to do:" << std::endl;
    std::cout << "1 - Delete user" << std::endl;
    std::cout << "2 - Delete some info of user" << std::endl;
    std::cin >> num;

    if (num == 1) {
        std::cout << "Enter client ID: ";
        std::cin >> client_id;

        con.prepare("delete_info", "DELETE FROM client_info WHERE id = $1");

        transaction.exec_prepared("delete_info", client_id);
        transaction.commit();
    } else if (num == 2) {
        std::string column_name;
        std::string column_value;

        std::cout << "Enter client ID: ";
        std::cin >> client_id;

        std::cout << "Enter column name to delete info from: ";
        std::cin >> column_name;

        con.prepare("delete_info", "UPDATE CASCADE client_info SET " + column_name + " = NULL WHERE id = $1");

        transaction.exec_prepared("delete_info", client_id);
        transaction.commit();
    } else {
        std::cout << "Wrong option" << std::endl;
    }
}

void find_info(pqxx::connection& con) {
	pqxx::work transaction(con);
	std::string search_value;
	int num;
	int client_id;

	std::cout << "Enter what you want do!" << std::endl;
	std::cout << "1-Find from First Name!" << std::endl;
	std::cout << "2-Find from Last Name!" << std::endl;
	std::cout << "3-Find from Email!" << std::endl;
	std::cout << "4-Find from Phone number!" << std::endl;
	std::cin >> num;


	std::string column_name;
	switch (num) {
	case 1:
		column_name = "first_name";
		break;
	case 2:
		column_name = "last_name";
		break;
	case 3:
		column_name = "email";
		break;
	case 4:
		column_name = "phone";
		break;
	default:
		std::cerr << "Wrong variant" << std::endl;
		return;
	}
	std::cout << "Enter the value to search for: ";
	std::cin >> search_value;
	
	std::string query = "SELECT DISTINCT client_info.*, phone_number.phone FROM client_info "
		"JOIN phone_number ON client_info.id = phone_number.client_id "
		"WHERE " + column_name +  " = $1 "
		"GROUP BY client_info.id, phone_number.phone";
	con.prepare("find_info", query);

	pqxx::result res = transaction.exec_prepared("find_info", search_value);

	// Вывод результатов запроса
	for (const auto& row : res) {
		std::cout << "ID: " << row["id"].as<int>() << std::endl;
		std::cout << "First Name: " << row["first_name"].as<std::string>() << std::endl;
		std::cout << "Last Name: " << row["last_name"].as<std::string>() << std::endl;
		std::cout << "Email: " << row["email"].as<std::string>() << std::endl;
		std::cout << "Phone Number: " << row["phone"].as<std::string>() << std::endl;
	}

	transaction.commit();
}


int main() {
	//setlocale(LC_ALL, "Russian");
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
	try
	{
		pqxx::connection con(
			"host= localhost"
			" port= 5432"
			" dbname= data_clients"
			" user= postgres"
			" password= 1234"
		);
		create_table(con);
		std::string think;

		std::cout << "Do you Want to do update(write up), delete(write del), add new client(add) or find some info(find): " << std::endl;
		std::cin >> think;
		if (think == "up") {
			update_info(con);
		}
		else if (think == "add") {
			add_client(con);
		}
		else if (think == "del") {
			delete_info(con);
		}
		else if (think == "find") {
			find_info(con);
		}

		else {
			std::cout << "you don't want change the info or add new client" << std::endl;
		}
	}
			
	catch (const std::exception& e)
	{
		std::cout << "Exception happened: " << e.what() << std::endl;
	}

	return 0;
}