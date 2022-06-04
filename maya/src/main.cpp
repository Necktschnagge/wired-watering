

#include "cpr/cpr.h"

#include <string>
#include <iostream>
#include <chrono>

#include <fstream>

static const std::string IP_ADDRESS_PUMP_SERVER_MAYSON{ "192.168.1.10" };
static const std::string IP_ADDRESS_VALVE_SERVER_JAMES{ "192.168.1.20" };
static const std::string IP_ADDRESS_VALVE_SERVER_LUCAS{ "192.168.1.21" };
static const std::string IP_ADDRESS_VALVE_SERVER_FELIX{ "192.168.1.22" };
static const std::string TEST_ADRESS_PING_FAIL{ "192.168.2.233" };

static constexpr uint8_t JAMES_GURKE_ERBSE{ 0b00000001 };
static constexpr uint8_t JAMES_TOMATE_ERDBEERE{ 0b00000010 };
static constexpr uint8_t JAMES_BOHNEN_ERDBEERE{ 0b00000100 };
static constexpr uint8_t JAMES_KAROTTEN{ 0b00001000 };

//static const std::string 

bool ping(const std::string& ip_address) {
	std::string bash_command{ "ping -c1 -s3 " };
	bash_command.append(ip_address);
	bash_command.append("  > /dev/null 2>&1");
#ifdef __linux__
	int x = system(bash_command.c_str());
	return (x == 0);
#else
	//cout << "ping skipped, not on linux" << endl;
	return false;
#endif // LINUX
}

int64_t get_seconds_since_epoch(bool verbose = false) {
	auto time_since_0 = std::chrono::system_clock::now().time_since_epoch();
	std::chrono::seconds seconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(time_since_0);

	if (verbose) {
		std::cout
			<< "std::chrono::system_clock::now().time_since_epoch()   :   "
			<< time_since_0.count()
			<< std::endl;
		std::cout
			<< "get_seconds_since_epoch(bool verbose /* true */)   :   "
			<< seconds_since_epoch.count()
			<< std::endl;
		std::cout
			<< "minutes since epoch   :   "
			<< (seconds_since_epoch.count() / 60)
			<< std::endl;
		std::cout
			<< "minute of day   :   "
			<< (seconds_since_epoch.count() / 60) % (24 * 60)
			<< std::endl;
		std::cout
			<< "minute of hour   :   "
			<< (seconds_since_epoch.count() / 60) % (60)
			<< std::endl;
		std::cout
			<< "hour of day   :   "
			<< (seconds_since_epoch.count() / 60 / 60) % (24)
			<< std::endl;
	}
	return seconds_since_epoch.count();
}

static uint8_t james_valves{ 0 };

void send_mayson(uint8_t auto_on = 2, uint8_t system_on = 2, uint8_t manual_on = 2) {
	std::string url{ "http://" };
	url += IP_ADDRESS_PUMP_SERVER_MAYSON;
	url += ":80/status";
	auto params = cpr::Parameters();
	if (auto_on < 2) params.Add({ "auto", auto_on == 1 ? "on" : "off" });
	if (system_on < 2) params.Add({ "system", system_on == 1 ? "on" : "off" });
	if (manual_on < 2) params.Add({ "manual", system_on == 1 ? "on" : "off" });

	cpr::Response r = cpr::Get(
		cpr::Url{ url },
		//cpr::Authentication{ "user", "pass", cpr::AuthMode::BASIC },
		params
	);
	//r.status_code;                  // 200
	//r.header["content-type"];       // application/json; charset=utf-8
	//r.text;
	std::cout
		<< "Send request to pump relay:\n"
		<< "URL:   " << r.url << std::endl;
	std::cout
		<< "Response:\n"
		<< "status code:   " << r.status_code << std::endl
		<< "text:\n" << r.text << std::endl;
}


void send_james(uint8_t valves) {
	std::cout << "send valves james\n";
	std::string url{ "http://" };
	url += IP_ADDRESS_VALVE_SERVER_JAMES;
	url += ":80/status";
	cpr::Response r = cpr::Get(
		cpr::Url{ url },
		//cpr::Authentication{ "user", "pass", cpr::AuthMode::BASIC },
		cpr::Parameters{ {"valves", std::to_string(valves) } }
	);
	//r.status_code;                  // 200
	//r.header["content-type"];       // application/json; charset=utf-8
	//r.text;

	std::cout << r.text << std::endl;
	std::cout << r.status_code << std::endl;

}

void apply_james(uint8_t valves_new) {
	uint8_t current_james{ james_valves };
	uint8_t turn_off = james_valves & (~valves_new);
	uint8_t turn_on = (~james_valves) & valves_new;
	for (uint8_t i = 0; i < 8; ++i) {
		uint8_t pos = 1 << i;
		if ((turn_off & pos) || (turn_on & pos)) {
			current_james &= ~pos;
			current_james |= pos & valves_new;
			send_james(current_james);
			std::this_thread::sleep_for(std::chrono::milliseconds(400));
		}
	}


	james_valves = valves_new;
	send_james(james_valves);
}

void watering(const int64_t& seconds_since_epoch) {
	const int64_t minutes_since_epoch{ seconds_since_epoch / 60 };
	const int64_t hours_since_epoch{ minutes_since_epoch / 60 };
	const int64_t days_since_epoch{ hours_since_epoch / 24 };

	//pumpe an
	send_mayson(0, 0, 0);

	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	send_mayson(1, 1);
	std::this_thread::sleep_for(std::chrono::seconds(6));

	if ((days_since_epoch % 2)) {

		auto start_watering_1 = get_seconds_since_epoch();
		apply_james(0b00000011);
		while (get_seconds_since_epoch() < start_watering_1 + 60 * 17) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		apply_james(0b00000000);

		auto start_watering_2 = get_seconds_since_epoch();
		apply_james(0b00001100);
		while (get_seconds_since_epoch() < start_watering_2 + 60 * 17) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		apply_james(0b00000000);

		//pumpe aus
	}
	else {
		auto start_watering_1 = get_seconds_since_epoch();
		apply_james(JAMES_GURKE_ERBSE);
		while (get_seconds_since_epoch() < start_watering_1 + 60 * 17) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		apply_james(0b00000000);

	}
	send_mayson(0);
}


int main(int argc, char** argv) {

	(void)argc;
	(void)argv;

	ping(IP_ADDRESS_PUMP_SERVER_MAYSON);
	ping(IP_ADDRESS_VALVE_SERVER_JAMES);

	auto seconds_since_epoch = get_seconds_since_epoch(true);
	auto minutes_since_epoch = (seconds_since_epoch / 60);
	auto minute_of_the_day = (seconds_since_epoch / 60) % (24 * 60);

	int64_t previous_timestamp = 0;
	{
		std::ifstream is;
		is.open("../../artifacts/timestamp.txt");
		if (is.is_open()) {
			std::string line;
			std::getline(is, line);
			try {
				previous_timestamp = std::stoll(line);
			}
			catch (...) {

			}
		}
		is.close();
	}
	std::cout << "previous time_stamp:   " << previous_timestamp << std::endl;

	if (
		(minute_of_the_day > (4 - 2) * 60) // 4:00 // -2 == UTC 
		&& (minute_of_the_day < (7 - 2) * 60) // 7:00
		&& (previous_timestamp + 3 * 60 + 1 < minutes_since_epoch) // 3 hours gone since last watering
		)
	{
		// save last timestamp:
		std::ofstream s;
		s.open("../../artifacts/timestamp.txt");
		if (s.good()) {
			s << minutes_since_epoch << std::endl;
			std::cout << "Wrote to timestamp.txt" << std::endl;

			watering(seconds_since_epoch);


		}
		else {
			std::cout << "error timestamp writing";
		}
	}

	return 0;
}
