
#include "logger.h"


#include "cpr/cpr.h"

#include <string>
#include <iostream>
#include <chrono>

#include <fstream>
#include <optional>

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

//static uint8_t james_valves{ 0 };

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

void send_valves(const std::string& ip_address, uint8_t valves, bool enable_log = true) {
	auto seconds = get_seconds_since_epoch();
	if (enable_log) std::cout << "Send valves at " << seconds << std::endl;
	std::string url{ "http://" };
	url += ip_address;
	url += ":80/status";
	cpr::Response r = cpr::Get(
		cpr::Url{ url },
		cpr::Parameters{ {"valves", std::to_string(valves) } }
	);
	if (enable_log) {
		std::cout << r.url << std::endl;
		std::cout << r.status_code << std::endl;
		std::cout << r.text << std::endl;
	}
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

void watering_helper(uint8_t valves, int64_t dur_sec) {
	send_valves(IP_ADDRESS_VALVE_SERVER_JAMES, valves);
	auto start_time = get_seconds_since_epoch();

	while (get_seconds_since_epoch() < start_time + dur_sec) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void watering(const int64_t& seconds_since_epoch) {
	const int64_t minutes_since_epoch{ seconds_since_epoch / 60 };
	const int64_t hours_since_epoch{ minutes_since_epoch / 60 };
	const int64_t days_since_epoch{ hours_since_epoch / 24 };

	//pumpe an
	send_mayson(0, 0, 0);

	std::this_thread::sleep_for(std::chrono::seconds(3));
	send_mayson(1, 1);
	std::this_thread::sleep_for(std::chrono::seconds(6));

	if ((days_since_epoch % 2)) {

		// GURK  0 : TOMA  0  : EMÖHR  0 : BOHN  0 // 0
		watering_helper(
			JAMES_GURKE_ERBSE | JAMES_TOMATE_ERDBEERE | JAMES_BOHNEN_ERDBEERE,
			60 * 5
		);
		// GURK  5 : TOMA  5  : EMÖHR  0 : BOHN  5 // 5
		watering_helper(
			JAMES_GURKE_ERBSE | JAMES_BOHNEN_ERDBEERE | JAMES_KAROTTEN,
			60 * 15
		);
		// GURK 20 : TOMA  5  : EMÖHR 15 : BOHN  20 // 20
		watering_helper(
			JAMES_TOMATE_ERDBEERE | JAMES_KAROTTEN,
			60 * 5
		);
		// GURK 20 : TOMA  10  : EMÖHR 20 : BOHN  20 // 25

		// restart pump
		watering_helper(JAMES_TOMATE_ERDBEERE | JAMES_GURKE_ERBSE, 1); // let water capacitor run dry while pump off
		send_mayson(0);
		std::this_thread::sleep_for(std::chrono::seconds(100));
		send_mayson(1);
		//restart pump end

		// GURK 20 : TOMA  10  : EMÖHR 20 : BOHN  20 // 25
		watering_helper(
			JAMES_GURKE_ERBSE | JAMES_BOHNEN_ERDBEERE,
			60 * 10
		);
		// GURK 30 : TOMA  10  : EMÖHR 20 : BOHN  30 // 35
		watering_helper(
			JAMES_TOMATE_ERDBEERE | JAMES_GURKE_ERBSE | JAMES_KAROTTEN,
			60 * 5
		);
		// GURK 35 : TOMA  15  : EMÖHR 25 : BOHN  30 // 40
		watering_helper(
			JAMES_BOHNEN_ERDBEERE | JAMES_KAROTTEN,
			60 * 15
		);
		// GURK 35 : TOMA  15  : EMÖHR 40 : BOHN  45 // 55
		watering_helper(
			JAMES_TOMATE_ERDBEERE | JAMES_KAROTTEN | JAMES_GURKE_ERBSE,
			60 * 5
		);
		// GURK 40 : TOMA  20  : EMÖHR 45 : BOHN  45 // 60 (whole time)
	}
	else {
		watering_helper(
			JAMES_GURKE_ERBSE,
			60 * 20
		);
		watering_helper(0, 30); // wait for pressure reached
		send_mayson(0); // restart pump
		std::this_thread::sleep_for(std::chrono::seconds(5*60)); // ... minutes pause
		send_mayson(1);
		watering_helper(
			JAMES_GURKE_ERBSE,
			60 * 20
		);
	}

	// valves off:
	send_valves(IP_ADDRESS_VALVE_SERVER_JAMES, 0);
	watering_helper(0, 30); // wait for pressure reached
	//pumpe aus
	send_mayson(0);

	// let capacitor run dry:
	watering_helper(
		JAMES_GURKE_ERBSE,
		60 * 3
	);
	send_valves(IP_ADDRESS_VALVE_SERVER_JAMES, 0);
}

#if false
class valve {
	std::string ip_address;
	std::string name;
	std::string label;
	uint8_t index;
};

class repetition_policy {
	// bestimmte wochentage (+x days)     1.1.1970 = Thursday(!)
	// 
	// bestimmte zahl an minuten 
	// allow overlapping...
};

class interrupted_watering_specification {

	std::chrono::seconds min_interval_length;

	double max_relative_protraction; // 1 == one interval watering
	double min_relative_protraction; // 0 .. 1 == everything is possible // 2 == watering for twice the time but only use 50% of the time.

};

class pressure_policy {

	double min_pressure;

	double max_pressure;

	std::optional<std::function<double(const double&)>> pressure_reward;

};



class watering_job {
	//end_time
	std::chrono::seconds time_to_finish;

	//duration
	std::chrono::seconds watering_duration;

	valve v;

	interrupted_watering_specification i;

	pressure_policy p;
	// enlongation_config
};


class timer {

	valve v;

	int64_t earliest_starting_time_s;
	int64_t legal_time_window_length_s;
	int64_t duration_time_sum_s;

	double max_interrupt_driven_duration_elongation_relative;
	double min_interrupt_driven_duration_elongation_relative;

	// sonder modus: nur in manuellem betrieb etc

	//wiederholung.
	repetition_policy p;

};

class schedule {

	std::vector<timer> all_timers;
};

#endif

int main(int argc, char** argv) {

	(void)argc;
	(void)argv;
	
	init_logger();

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
