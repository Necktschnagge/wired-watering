#pragma once

#include "config.h"

#include <string>

bool get_server_answer_string(char* first_char, size_t length) {
	if (length == 0) return false;
	size_t max_string_length = length - 1;

	std::string server_answer;
	server_answer = "{\n \"server-name\" : \"";
	server_answer.append(string_server_name);
	server_answer.append("\",\n \"action\" : \"okay\"\n}");

	if (server_answer.length() > max_string_length) return false;
	std::copy(server_answer.cbegin(), server_answer.cend(), first_char);
	first_char[server_answer.length()] = '\0';

	return true;
}

bool get_pressure_answer(char* first_char, size_t length, uint16_t raw_pressure) {
	if (length == 0) return false;
	size_t max_string_length = length - 1;

	std::string server_answer;
	server_answer = "{\n \"server-name\" : \"";
	server_answer.append(string_server_name);
	server_answer.append("\",\n \"raw-pressure\" : ");
	server_answer.append(std::to_string(raw_pressure));
	server_answer.append("\n }");

	if (server_answer.length() > max_string_length) return false;
	std::copy(server_answer.cbegin(), server_answer.cend(), first_char);
	first_char[server_answer.length()] = '\0';

	return true;
}

bool get_relay_answer_string(char* first_char, size_t length) {
	//const char* resp_str = "{\n \"server-name\" : \"pump-relay-mayson\"\n \"manual\" : XXXXX\n \"auto\" : XXXXX\n \"system\" : XXXXX\n}";

	if (length == 0) return false;
	size_t max_string_length = length - 1;

	std::string server_answer;
	server_answer = "{\n \"server-name\" : \"";
	server_answer.append(string_server_name);
	server_answer.append("\",\n \"action\" : \"okay\"\n}");

	if (server_answer.length() > max_string_length) return false;
	std::copy(server_answer.cbegin(), server_answer.cend(), first_char);
	first_char[server_answer.length()] = '\0';

	return true;
}

