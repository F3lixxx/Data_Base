#include <iostream>
#include "pqxx/pqxx"
#include "Windows.h"

void create_table(pqxx::connection &con){
	pqxx::work transaction(con);

		transaction.exec(
			"CREATE TABLE IF NOT EXISTS client_info ("
			"id SERIAL PRIMARY KEY NOT NULL,"
			"first_name TEXT,"
			"last_name TEXT,"
			"email VARCHAR)");
		transaction.exec(
			"CREATE TABLE IF NOT EXISTS phone_number("
			"client_id INTEGER ,"
			"phone TEXT, "
			"FOREIGN KEY (client_id) REFERENCES client_info(id) ON DELETE CASCADE)");
		transaction.commit();
}

void add_client(pqxx::connection& con) {
	pqxx::work trans(con);

	std::cout << "Enter First Name: ";
	std::string first_name;
	std::cin >> first_name;

	std::cout << "Enter Last Name: ";
	std::string last_name;
	std::cin >> last_name;

	std::cout << "Enter Email: ";
	std::string email;
	std::cin >> email;

		pqxx::result res = trans.exec_prepared("add_client", first_name, last_name, email);
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

void update_info(pqxx::connection& con, std::string& update_column_name) {
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
	if (num == 1) {
		column_name = "first_name";
	}
	else if (num == 2) {
		column_name = "last_name";
	}
	else if (num == 3) {
		column_name = "email";
	}
	else if (num == 4) {
		column_name = "phone";
	}
	else {
		std::cerr << "Wrong variant" << std::endl;
		return;
	}

	std::cout << "Enter client ID: ";
	std::cin >> client_id;

	if (num == 4) {
		std::string number;
		std::cout << "Enter number what you want update: ";
		std::cin >> number;

		transaction.exec_prepared("update_info_phone", new_value, client_id, number);
	}
	else {
		con.prepare("update_info_client", "UPDATE client_info SET " + column_name + " = $1 WHERE id = $2");
		transaction.exec_prepared("update_info_client", new_value, client_id);
	}

	transaction.commit();
}

void delete_info(pqxx::connection& con, std::string column_name) {
	pqxx::work transaction(con);
	int num;
	int client_id;

	std::cout << "Enter what you want to do:" << std::endl;
	std::cout << "1 - Delete user" << std::endl;
	std::cout << "2 - Delete phone" << std::endl;
	std::cin >> num;

	if (num == 1) {
		std::cout << "Enter client ID: ";
		std::cin >> client_id;

		transaction.exec_prepared("delete_user", client_id);
	}
	else if (num == 2) {
		std::cout << "Enter client ID: ";
		std::cin >> client_id;

			std::string number;
			std::cout << "Enter number what you want delete: ";
			std::cin >> number;
			transaction.exec_prepared("delete_phone", number);
		}

		transaction.commit();
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
		"WHERE " + column_name + " = $1 "
		"GROUP BY client_info.id, phone_number.phone";
	con.prepare("find_info", query);

	pqxx::result res = transaction.exec_prepared("find_info", search_value);

	bool first_iteration = true; // Флаг, чтобы определить, что это первая итерация

	for (const auto& row : res) {
		if (first_iteration) { // Если это первая итерация, выводим информацию о пользователе
			std::cout << "ID: " << row["id"].as<int>() << std::endl;
			std::cout << "First Name: " << row["first_name"].as<std::string>() << std::endl;
			std::cout << "Last Name: " << row["last_name"].as<std::string>() << std::endl;
			std::cout << "Email: " << row["email"].as<std::string>() << std::endl;

			first_iteration = false; // Устанавливаем флаг первой итерации в false
		}
		transaction.commit();
	}
	std::cout << "Email: ";
	for (const auto& row : res) {
		std::string phone_numbers_str = row["phone"].as<std::string>();
		std::istringstream iss(phone_numbers_str);
		std::vector<std::string> phone_numbers_vec{ std::istream_iterator<std::string>{iss},
												   std::istream_iterator<std::string>{} };


		for (int i = 0; i < phone_numbers_vec.size(); ++i) {
			std::cout << phone_numbers_vec[i] << ' ';
		}
		transaction.commit();
	}
}


int main() {
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

		std::string column_name;
		

		con.prepare("add_client", "INSERT INTO client_info(first_name, last_name, email) VALUES ($1, $2, $3) RETURNING id");
		con.prepare("insert_phone", "INSERT INTO phone_number(client_id, phone) VALUES($1, $2)");
		con.prepare("delete_user", "DELETE FROM client_info WHERE id = $1");
		con.prepare("delete_phone", "DELETE FROM phone_number WHERE phone = $1");
		con.prepare("update_info_phone", "UPDATE phone_number SET phone = $1 WHERE client_id = $2 AND phone = $3");


		for (std::string column_name : { "first_name", "last_name", "email" }) {
			con.prepare("update_info_" + column_name, "UPDATE client_info SET " + column_name + " = $1 WHERE id = $2");
			con.prepare("delete_" + column_name, "UPDATE client_info SET " + column_name + " = $1 WHERE id = $2");
		}

		create_table(con);
		std::string think;

		std::cout << "Do you Want to do update(write up), delete(write del), add new client(add) or find some info(find): " << std::endl;
		std::cin >> think;
		if (think == "up") {
			update_info(con, column_name);
		}
		else if (think == "add") {
			add_client(con);
		}
		else if (think == "del") {
			delete_info(con, column_name);
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