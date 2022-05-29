

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
//static const std::string 

bool ping(const std::string& ip_address) {
	using std::cout, std::endl;

	std::string bash_command{ "ping -c1 -s1 " };
	bash_command.append(ip_address);
	bash_command.append("  > /dev/null 2>&1");
#ifdef __linux__
	int x = system(bash_command.c_str());
	cout << "ping on linux" << endl;
#else
	int x = 0;
	cout << "ping skipped, not on linux" << endl;
#endif // LINUX
	if (x == 0) {
		cout << "ping success" << endl;
		return true;
	}
	else {
		cout << "ping failed" << endl;
		return false;
	}
}

int64_t get_seconds_since_epoch() {
	auto time_since_0 = std::chrono::system_clock::now().time_since_epoch();

	std::cout << time_since_0.count() << std::endl;
	auto seconds = time_since_0.count() / 10000000;
#ifdef __linux__
	seconds /= 100;
#endif
	std::cout << seconds << std::endl; //two more 0 on linux
	std::cout << (seconds / 60) % (24 * 60) << std::endl; // hour of the day in UTC (we are 2 hours ahead.)

	return seconds;
}

static uint8_t james_valves{ 0 };

void send_mayson(bool on) {
	std::cout << "send pump relay\n" << on << std::endl;
	std::string url{ "http://" };
	url += IP_ADDRESS_PUMP_SERVER_MAYSON;
	url += ":80/status";
	cpr::Response r = cpr::Get(
		cpr::Url{ url },
		//cpr::Authentication{ "user", "pass", cpr::AuthMode::BASIC },
		cpr::Parameters{ {"auto", on ? "on" : "off" }}
	);
	//r.status_code;                  // 200
	//r.header["content-type"];       // application/json; charset=utf-8
	//r.text;

	std::cout << r.text << std::endl;
	std::cout << r.status_code << std::endl;

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


	if (!(days_since_epoch % 2)) {
		
		//pumpe an
		send_mayson(true);
		
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
		
		send_mayson(false);
		//pumpe aus
	}
}


int main(int argc, char** argv) {

	(void)argc;
	(void)argv;


	ping("1.1.1.1");
	ping(IP_ADDRESS_PUMP_SERVER_MAYSON);
	ping(TEST_ADRESS_PING_FAIL);

	cpr::Response r = cpr::Get(
		cpr::Url{ "https://api.github.com/repos/libcpr/cpr/contributors" },
		//cpr::Authentication{ "user", "pass", cpr::AuthMode::BASIC },
		cpr::Parameters{ {"anon", "true"}, {"key", "value"} }
	);
	//r.status_code;                  // 200
	//r.header["content-type"];       // application/json; charset=utf-8
	//r.text;

	std::cout << r.text << std::endl;
	std::cout << r.status_code << std::endl;

	auto seconds = get_seconds_since_epoch();
	auto minutes_since_epoch = (seconds / 60);
	auto minute_of_the_day = (seconds / 60) % (24 * 60);

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

			watering(seconds);


		}
		else {
			std::cout << "error timestamp writing";
		}
	}

	return 0;
}
