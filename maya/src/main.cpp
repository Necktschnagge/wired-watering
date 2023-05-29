
#include "logger.h"


#include "cpr/cpr.h"

#include <string>
#include <iostream>
#include <chrono>

#include <fstream>
#include <optional>


[[maybe_unused]] static const std::string IP_ADDRESS_PUMP_SERVER_MAYSON{ "192.168.1.10" };

[[maybe_unused]] static const std::string IP_ADDRESS_VALVE_SERVER_JAMES{ "192.168.1.20" };
[[maybe_unused]] static const std::string IP_ADDRESS_VALVE_SERVER_LUCAS{ "192.168.1.21" };
[[maybe_unused]] static const std::string IP_ADDRESS_VALVE_SERVER_FELIX{ "192.168.1.22" };

[[maybe_unused]] static const std::string IP_ADDRESS_VALVE_SERVER_TEST{ "192.168.1.23" };

[[maybe_unused]] static const std::string TEST_ADRESS_PING_FAIL{ "192.168.2.233" };

[[maybe_unused]] static constexpr uint8_t JAMES_VALVE_1{ 0b00000001 };
[[maybe_unused]] static constexpr uint8_t JAMES_VALVE_2{ 0b00000010 };
[[maybe_unused]] static constexpr uint8_t JAMES_VALVE_3{ 0b00000100 };
[[maybe_unused]] static constexpr uint8_t JAMES_VALVE_4{ 0b00001000 };

[[maybe_unused]] static constexpr uint8_t LUCAS_VALVE_1{ 0b00000001 };
[[maybe_unused]] static constexpr uint8_t LUCAS_VALVE_2{ 0b00000010 };
[[maybe_unused]] static constexpr uint8_t LUCAS_VALVE_3{ 0b00000100 };

[[maybe_unused]] static constexpr uint8_t FELIX_VALVE_1{ 0b00000001 };
[[maybe_unused]] static constexpr uint8_t FELIX_VALVE_2{ 0b00000010 };

[[maybe_unused]] static constexpr uint8_t JAMES_GURKE_ERBSE{ JAMES_VALVE_1 };
[[maybe_unused]] static constexpr uint8_t JAMES_TOMATE{ JAMES_VALVE_2 };
[[maybe_unused]] static constexpr uint8_t JAMES_BOHNEN{ JAMES_VALVE_4 };

//[[maybe_unused]] static constexpr uint8_t LUCAS_KAROTTEN{ LUCAS_VALVE_1 };
//[[maybe_unused]] static constexpr uint8_t LUCAS_ERDBEEREN{ LUCAS_VALVE_2 };
[[maybe_unused]] static constexpr uint8_t LUC_NEUE_ERDBEEREN_AN_DER_ROSE{ LUCAS_VALVE_1 };
[[maybe_unused]] static constexpr uint8_t LUC_HEIDELBEEREN{ LUCAS_VALVE_2 };
[[maybe_unused]] static constexpr uint8_t LUC_KARTOFFELN_UND_ERDBEEREN{ LUCAS_VALVE_3 };

[[maybe_unused]] static constexpr uint8_t FELIX_MARA{ FELIX_VALVE_2 };
[[maybe_unused]] static constexpr uint8_t FELIX_EIBEN{ FELIX_VALVE_1 };

//static const std::string 

bool ping(const std::string& ip_address) {
#if defined(__linux__) || defined(__APPLE__)
	std::string bash_command{ "ping -c1 -s3 " };
	bash_command.append(ip_address);
	bash_command.append("  > /dev/null 2>&1");
	int x = system(bash_command.c_str());
	return (x == 0);
#endif // defined(__linux__) || defined(__APPLE__)
#ifdef _WIN32
	std::string batch_command{ "ping -n 1 " };
	batch_command.append(ip_address);
	batch_command.append("  > nul 2>&1");
	int x = system(batch_command.c_str());
	return (x == 0);
#endif // _WIN32
}

class time_helper {
public:
	static int64_t get_seconds_since_epoch_now(bool verbose = false) {
		auto time_since_0 = std::chrono::system_clock::now().time_since_epoch();

		std::chrono::seconds seconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(time_since_0);

		if (verbose) {
			std::cout
				<< "std::chrono::system_clock::now().time_since_epoch()   :   "
				<< time_since_0.count()
				<< std::endl;
			std::cout
				<< "get_seconds_since_epoch_now(bool verbose /* true */)   :   "
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
private:
	int64_t seconds_since_epoch;
public:

	time_helper(bool verbose = false) : seconds_since_epoch(get_seconds_since_epoch_now(verbose)) {}

	time_helper(int64_t seconds_since_epoch) : seconds_since_epoch(seconds_since_epoch) {}

	inline int64_t get_seconds_since_epoch() const {
		return seconds_since_epoch;
	}
	inline int64_t get_minutes_since_epoch() const {
		return (get_seconds_since_epoch() / 60);
	}
	inline int64_t get_hours_since_epoch() const {
		return (get_minutes_since_epoch() / 60);
	}
	inline int64_t get_days_since_epoch() const {
		return (get_hours_since_epoch() / 24);
	}

	inline int64_t get_minute_intra_day() const {
		return get_minutes_since_epoch() % (24 * 60);
	}

};


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
	auto seconds = time_helper::get_seconds_since_epoch_now();
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

[[deprecated]] void send_james(uint8_t valves) {
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

void wait_for(int64_t duration_in_seconds) {
	auto start_time = time_helper::get_seconds_since_epoch_now();

	while (time_helper::get_seconds_since_epoch_now() < start_time + duration_in_seconds) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void watering(const time_helper& start_time) {

	//pumpe an
	send_mayson(0, 0, 0);

	std::this_thread::sleep_for(std::chrono::seconds(3));
	send_mayson(1, 1);
	std::this_thread::sleep_for(std::chrono::seconds(6));

	if (start_time.get_days_since_epoch() % 2 == 1) {

		send_valves(IP_ADDRESS_VALVE_SERVER_JAMES, JAMES_VALVE_1); // carrot
		send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, LUC_KARTOFFELN_UND_ERDBEEREN); // potato

		wait_for(10 * 60);

		send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, FELIX_MARA); // mara

		wait_for(10 * 60);

		send_mayson(0);
		wait_for(10);
		send_mayson(1);

		send_valves(IP_ADDRESS_VALVE_SERVER_JAMES, 0); // carrot off
		send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, FELIX_MARA | FELIX_EIBEN); // eibe

		wait_for(10 * 60);

		send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, FELIX_EIBEN); // mara off

		wait_for(25 * 60);

		send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, 0); // potato off
		send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, 0); // eibe off
		send_valves(IP_ADDRESS_VALVE_SERVER_JAMES, 0);
	}

	send_mayson(0);
	wait_for(10);
	send_mayson(1);

	const uint8_t LUCAS_ADD_BLUEBERRIES{ (start_time.get_days_since_epoch() % 4 == 0) ? LUC_HEIDELBEEREN : uint8_t(0) };

	send_valves(IP_ADDRESS_VALVE_SERVER_JAMES, JAMES_VALVE_2 | JAMES_VALVE_3); // cucumber + tomato
	send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, LUC_NEUE_ERDBEEREN_AN_DER_ROSE | LUCAS_ADD_BLUEBERRIES); // new stawberries

	wait_for(15 * 60);

	send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, LUCAS_ADD_BLUEBERRIES);
	send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, 0);
	send_valves(IP_ADDRESS_VALVE_SERVER_JAMES, JAMES_VALVE_2);

	wait_for(15 * 60);

	send_mayson(0);
	wait_for(10);
	send_mayson(1);

	wait_for(30 * 60);




#if false
	if ((days_since_epoch % 2)) {

		// TOMA - Mara simultan
		// GURK  0 : TOMA  0  : ERDBE  0 : BOHN  0 // 0
		send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, 0);
		send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, FELIX_MARA);
		send_valves(
			IP_ADDRESS_VALVE_SERVER_JAMES,
			JAMES_GURKE_ERBSE | JAMES_TOMATE | JAMES_BOHNEN
		);
		wait_for(60 * 5);
		// GURK  5 : TOMA  5  : ERDBE  0 : BOHN  5 // 5
		send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, LUCAS_ERDBEEREN);
		send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, 0);
		send_valves(
			IP_ADDRESS_VALVE_SERVER_JAMES,
			JAMES_GURKE_ERBSE | JAMES_BOHNEN
		);
		wait_for(60 * 15);
		// GURK 20 : TOMA  5  : ERDBE 15 : BOHN  20 // 20
		send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, LUCAS_ERDBEEREN);
		send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, FELIX_MARA);
		send_valves(
			IP_ADDRESS_VALVE_SERVER_JAMES,
			JAMES_TOMATE
		);
		wait_for(60 * 5);
		// GURK 20 : TOMA  10  : ERDBE 20 : BOHN  20 // 25

		// restart pump
		send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, 0);
		send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, FELIX_MARA);
		send_valves(
			IP_ADDRESS_VALVE_SERVER_JAMES,
			JAMES_TOMATE | JAMES_GURKE_ERBSE
		); // let water capacitor run dry while pump off
		send_mayson(0);
		std::this_thread::sleep_for(std::chrono::seconds(100));
		send_mayson(1);
		//restart pump end

		// GURK 20 : TOMA  10  : ERDBE 20 : BOHN  20 // 25
		send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, 0);
		send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, 0);
		send_valves(
			IP_ADDRESS_VALVE_SERVER_JAMES,
			JAMES_GURKE_ERBSE | JAMES_BOHNEN
		);
		wait_for(60 * 10);
		// GURK 30 : TOMA  10  : ERDBE 20 : BOHN  30 // 35
		send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, LUCAS_ERDBEEREN);
		send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, FELIX_MARA);
		send_valves(
			IP_ADDRESS_VALVE_SERVER_JAMES,
			JAMES_TOMATE | JAMES_GURKE_ERBSE
		);
		wait_for(60 * 5);
		// GURK 35 : TOMA  15  : ERDBE 25 : BOHN  30 // 40
		send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, LUCAS_ERDBEEREN);
		send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, 0);
		send_valves(
			IP_ADDRESS_VALVE_SERVER_JAMES,
			JAMES_BOHNEN
		);
		wait_for(60 * 15);
		// GURK 35 : TOMA  15  : ERDBE 40 : BOHN  45 // 55
		send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, LUCAS_ERDBEEREN);
		send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, FELIX_MARA);
		send_valves(
			IP_ADDRESS_VALVE_SERVER_JAMES,
			JAMES_TOMATE | JAMES_GURKE_ERBSE
		);
		wait_for(60 * 5);
		// GURK 40 : TOMA  20  : ERDBE 45 : BOHN  45 // 60 (whole time)
	}
	else {
		send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, LUCAS_KAROTTEN);
		send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, FELIX_EIBEN);
		send_valves(IP_ADDRESS_VALVE_SERVER_JAMES, JAMES_GURKE_ERBSE);
		wait_for(60 * 20);

		// wait for pressure reached
		send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, 0);
		send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, 0);
		send_valves(IP_ADDRESS_VALVE_SERVER_JAMES, 0);
		wait_for(30);

		send_mayson(0); // restart pump
		std::this_thread::sleep_for(std::chrono::seconds(5 * 60)); // ... minutes pause
		send_mayson(1);

		send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, LUCAS_KAROTTEN);
		send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, FELIX_EIBEN);
		send_valves(IP_ADDRESS_VALVE_SERVER_JAMES, JAMES_GURKE_ERBSE);
		wait_for(60 * 20);
	}
#endif
	// just to demonstrate:
	//send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, FELIX_EIBEN);
	//wait_for(60 * 3);

	// valves off:
	send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, 0);
	send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, 0);
	send_valves(IP_ADDRESS_VALVE_SERVER_JAMES, 0);
	// wait for pressure reached
	wait_for(30);
	//pumpe aus
	send_mayson(0);

	// let capacitor run dry:
	send_valves(
		IP_ADDRESS_VALVE_SERVER_FELIX,
		FELIX_EIBEN
	);
	wait_for(60 * 2);

	send_valves(IP_ADDRESS_VALVE_SERVER_LUCAS, 0);
	send_valves(IP_ADDRESS_VALVE_SERVER_FELIX, 0);
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

bool check_all_servers_using_ping() {

	standard_logger()->info("Pinging Pump Server Mayson...");

	const bool MAYSON_AVAILABLE{ ping(IP_ADDRESS_PUMP_SERVER_MAYSON) };
	if (!MAYSON_AVAILABLE) {
		standard_logger()->error("Fatal: Pump Server not available!");
	}
	else {
		standard_logger()->info("Pump Server ping OK!");
	}

	standard_logger()->info("Pinging Pump Server Mayson   ...DONE!");

	standard_logger()->info("Pinging all valve stations...");
	const bool JAMES_AVAILABLE{ ping(IP_ADDRESS_VALVE_SERVER_JAMES) };
	const bool LUCAS_AVAILABLE{ ping(IP_ADDRESS_VALVE_SERVER_LUCAS) };
	const bool FELIX_AVAILABLE{ ping(IP_ADDRESS_VALVE_SERVER_FELIX) };

	if (!JAMES_AVAILABLE) {
		standard_logger()->error("Fatal: James not available!");
	}
	else {
		standard_logger()->info("James ping OK!");
	}
	if (!LUCAS_AVAILABLE) {
		standard_logger()->error("Fatal: Lucas not available!");
	}
	else {
		standard_logger()->info("Lucas ping OK!");
	}
	if (!FELIX_AVAILABLE) {
		standard_logger()->error("Fatal: Felix not available!");
	}
	else {
		standard_logger()->info("Felix ping OK!");
	}

	standard_logger()->info("Pinging all valve stations   ...DONE!");


	standard_logger()->info("Pinging Non-Existing Test Server...");

	const bool TEST_SERVER_UNAVAILABLE{ !ping(IP_ADDRESS_VALVE_SERVER_TEST) };

	if (!TEST_SERVER_UNAVAILABLE) {
		standard_logger()->error("Got successful PING from Device that should not exists on local network!");
	}
	else {
		standard_logger()->info("Pinging Non-Existing Test Server OK!");
	}

	standard_logger()->info("Pinging Non-Existing Test Server   ...DONE!");

	return MAYSON_AVAILABLE && JAMES_AVAILABLE && LUCAS_AVAILABLE && FELIX_AVAILABLE && TEST_SERVER_UNAVAILABLE;
}

int64_t load_timestamp_file() {
	standard_logger()->info("Checking timestamp.txt...");
	standard_logger()->debug("The timestamp.txt is always written when watering is triggered. It is read to check if I was already done with watering some minutes ago.");
	int64_t previous_timestamp = 0;
	{
		std::ifstream is;
		is.open("../../artifacts/timestamp.txt");
		if (is.is_open()) {
			std::string line;
			std::getline(is, line); // does this throw?
			try {
				previous_timestamp = std::stoll(line);
			}
			catch (...) {
				standard_logger()->error("Cannot convert content of timestamp.txt to \"long long\" (std::stoll)");
			}
		}
		else {
			standard_logger()->error("Cannot open timestamp.txt");
		}
		is.close();
	}
	if (previous_timestamp == 0) {
		standard_logger()->warn("Got previous_timestamp == 0. This may be due to missing timestamp.txt");
	}
	std::cout << "previous time_stamp:   " << previous_timestamp << std::endl;
	standard_logger()->info("Checking timestamp.txt   ...DONE!");

	return previous_timestamp;
}

bool check_if_in_watering_time_window(const time_helper& start_time, int64_t previous_timestamp) {
	bool result{ false };
	if (
		(start_time.get_minute_intra_day() > (3 - 2) * 60 + 30) // 3:30 // -2 == UTC 
		&& (start_time.get_minute_intra_day() < (6 - 2) * 60 + 30) // 6:30
		&& (previous_timestamp + 3 * 60 + 1 < start_time.get_minutes_since_epoch()) // 3 hours gone since last watering
		)
	{
		// save last timestamp:
		std::ofstream s;
		s.open("../../artifacts/timestamp.txt");
		if (s.good()) {
			s << start_time.get_minutes_since_epoch() << std::endl;
			std::cout << "Wrote to timestamp.txt" << std::endl;
			result = true;
		}
		else {
			std::cout << "error timestamp writing";
		}
	}
	else {
		standard_logger()->info("It's not time for watering now!");
		standard_logger()->info(std::string("Minute Intraday:   ") + std::to_string(start_time.get_minute_intra_day()));
		standard_logger()->info(std::string("Days since Epoch:   ") + std::to_string(start_time.get_days_since_epoch()));
	}

	return result;
}

int main(int argc, char** argv) {

	(void)argc;
	(void)argv;

	init_logger();

	for (std::size_t i{ 0 }; i < 20; ++i) {
		const bool PING_OK{ check_all_servers_using_ping() };
		if (PING_OK)
			break;
	}

	standard_logger()->info("Fetching timestamp...");
	const auto start_time = time_helper(true);

	const int64_t previous_timestamp = load_timestamp_file();

	const bool is_time_for_watering = check_if_in_watering_time_window(start_time, previous_timestamp);

	constexpr bool global_watering_enable{ true };

	if (global_watering_enable && is_time_for_watering) {
		watering(start_time);
	}


	return 0;
}
