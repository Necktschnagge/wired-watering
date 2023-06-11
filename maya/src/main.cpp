
#include "logger.h"


#include "cpr/cpr.h"

#include <vector>
#include <string>
#include <iostream>
#include <chrono>

#include <fstream>
#include <optional>

static constexpr bool MANUAL_TEST{ false };

namespace CONF {


	// server names / device names:
	[[maybe_unused]] inline static const std::string FELIX{ "Felix" };
	[[maybe_unused]] inline static const std::string JAMES{ "James" };
	[[maybe_unused]] inline static const std::string LUCAS{ "Lucas" };
	[[maybe_unused]] inline static const std::string MAYSON{ "Mayson" };


	// valve patch infos:
	[[maybe_unused]] inline static const std::string FELIX_VALVE_1_LABEL{ "Eiben Klee" };
	[[maybe_unused]] inline static const std::string FELIX_VALVE_2_LABEL{ "Mara Sabine" };

	[[maybe_unused]] inline static const std::string JAMES_VALVE_1_LABEL{ "Karotten" };
	[[maybe_unused]] inline static const std::string JAMES_VALVE_2_LABEL{ "Gurken" };
	[[maybe_unused]] inline static const std::string JAMES_VALVE_3_LABEL{ "Tomaten" };
	[[maybe_unused]] inline static const std::string JAMES_VALVE_4_LABEL{ "-frei-" };

	[[maybe_unused]] inline static const std::string LUCAS_VALVE_1_LABEL{ "Erdbeerren Flieder" };
	[[maybe_unused]] inline static const std::string LUCAS_VALVE_2_LABEL{ "Heidelbeeren" };
	[[maybe_unused]] inline static const std::string LUCAS_VALVE_3_LABEL{ "Kartofffeln" };


	// ip addresses:

}

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


[[maybe_unused]] static constexpr uint8_t JAMES_KAROTTEN{ JAMES_VALVE_1 };
[[maybe_unused]] static constexpr uint8_t JAMES_TOMATEN{ JAMES_VALVE_2 };
[[maybe_unused]] static constexpr uint8_t JAMES_GURKEN{ JAMES_VALVE_3 };
[[maybe_unused]] static constexpr uint8_t JAMES_FREI{ JAMES_VALVE_4 };

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
	std::string batch_command{ "Test-Connection -Count 1 -ErrorAction Stop " };
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



namespace k1 {

	class valve_station {
		std::string label;
		std::string ip_address;
		std::vector<std::string> valves;
		std::vector<std::chrono::duration<double, std::milli>> accumulated_ms;
		//std::vector<std::chrono::time_point<std::chrono::steady_clock>> last_switching_time;
		std::chrono::time_point<std::chrono::steady_clock> last_switching_time;

		uint8_t valves_state;

		friend class landscape;
		friend class valve_view;


		void send(bool enable_log = true) {
			send_valves(ip_address, valves_state, enable_log);
		}

		void turn(bool on, uint8_t valve_bit_mask) {
			standard_logger()->info("-------------------------------------");
			const uint8_t old_state{ valves_state };
			if (on)
				valves_state |= valve_bit_mask;
			else
				valves_state &= ~valve_bit_mask;
			send();
			const auto now = std::chrono::steady_clock::now();
			for (uint8_t i = 0; i < accumulated_ms.size(); ++i) {
				if (old_state & (static_cast<uint8_t>(1) << i)) {
					accumulated_ms[i] += now - last_switching_time;
				}
			}
			std::string message{
				"Switched the following valves: --->\n"
			};

			for (uint8_t i = 0; i < valves.size(); ++i) {
				const uint8_t mask{ static_cast<uint8_t>(uint8_t(1) << i) };
				const bool switched = (old_state & mask) != (valves_state & mask);
				const std::string next_string = (valves_state & mask) ? "ON" : "OFF";
				const std::string prev_string = (valves_state & mask) ? "OFF" : "ON";
				if (switched)
					message += label + ":   " + valves.at(i) + "   " + prev_string + " -> " + next_string;
			}
			message +=
				"                                   <---";

			standard_logger()->info(message);

			last_switching_time = now;
		}

		inline uint8_t mask_all() {
			uint8_t mask{ 0 };
			for (std::size_t i = 0; i < valves.size(); ++i) {
				mask += uint8_t(1) << i;
			}
			return mask;
		}

		inline void turn_all(bool on) {
			return turn(on, mask_all());
		}

		valve_station(const std::string& _label, const std::string& _ip_address, std::vector<std::string>&& _valves) :
			label(_label),
			ip_address(_ip_address),
			valves(std::move(_valves)),
			valves_state(0)
		{
			accumulated_ms = std::vector<std::chrono::duration<double, std::milli>>(valves.size(), std::chrono::duration<double, std::milli>(0.0));
			//last_switching_time = std::vector<std::chrono::time_point<std::chrono::steady_clock>>(valves.size(), std::chrono::steady_clock::now());
			last_switching_time = std::chrono::steady_clock::now();
		}

	public:

		valve_station(valve_station&&) = default;
		valve_station(const valve_station&) = delete;

		valve_station& operator = (const valve_station&) = delete;
		valve_station& operator = (valve_station&&) = delete;


		class valve_view {
			valve_station& station;
			uint8_t valve_bit_mask;

			valve_view(valve_station& _station, uint8_t _valve_bit_mask) : station(_station), valve_bit_mask(_valve_bit_mask) {

			}
			friend class valve_station;

		public:
			valve_view(const valve_view&) = default;
			valve_view(valve_view&&) = default;


			inline void turn(bool on) const {
				return station.turn(on, valve_bit_mask);
			}

			inline void turn_on() const {
				return turn(true);
			}

			inline void turn_off() const {
				return turn(false);
			}

		};

		valve_view get_view(uint8_t valve_bit_mask) {
			//(void)valves.at(valve_id); // check out of range!
			return valve_view(*this, valve_bit_mask);
		}

	};



	class landscape {
		std::vector<valve_station> stations;
		static std::optional<landscape> singleton_instance;


		landscape() {
			standard_logger()->info("Creating Landscape...");

			standard_logger()->info("Creating Felix...");
			stations.push_back(valve_station(CONF::FELIX, IP_ADDRESS_VALVE_SERVER_FELIX, std::vector<std::string>{ CONF::FELIX_VALVE_1_LABEL, CONF::FELIX_VALVE_2_LABEL}));

			standard_logger()->info("Creating James...");
			stations.push_back(valve_station(CONF::JAMES, IP_ADDRESS_VALVE_SERVER_JAMES, std::vector<std::string>{ CONF::JAMES_VALVE_1_LABEL, CONF::JAMES_VALVE_2_LABEL, CONF::JAMES_VALVE_3_LABEL, CONF::JAMES_VALVE_4_LABEL}));

			standard_logger()->info("Creating Lucas...");
			stations.push_back(valve_station(CONF::LUCAS, IP_ADDRESS_VALVE_SERVER_LUCAS, std::vector<std::string>{ CONF::LUCAS_VALVE_1_LABEL, CONF::LUCAS_VALVE_2_LABEL, CONF::LUCAS_VALVE_3_LABEL}));
		}

	public:

		class felix_view {
			valve_station& station;

			friend class landscape;

			felix_view(valve_station& _station) : station(_station) {}

		public:

			valve_station::valve_view Eiben() { return station.get_view(FELIX_EIBEN); }
			valve_station::valve_view MaraSabine() { return station.get_view(FELIX_MARA); }

			inline void turn(bool on) const {
				return station.turn_all(on);
			}

			inline void turn_on() const {
				return turn(true);
			}

			inline void turn_off() const {
				return turn(false);
			}
		};

		class james_view {
			valve_station& station;

			friend class landscape;

			james_view(valve_station& _station) : station(_station) {}

		public:

			valve_station::valve_view Karotten() { return station.get_view(JAMES_KAROTTEN); }
			valve_station::valve_view Gurken() { return station.get_view(JAMES_GURKEN); }
			valve_station::valve_view Tomaten() { return station.get_view(JAMES_TOMATEN); }
			valve_station::valve_view Frei() { return station.get_view(JAMES_FREI); }

			inline void turn(bool on) const {
				return station.turn_all(on);
			}

			inline void turn_on() const {
				return turn(true);
			}

			inline void turn_off() const {
				return turn(false);
			}

		};

		class lucas_view {
			valve_station& station;

			friend class landscape;

			lucas_view(valve_station& _station) : station(_station) {}

		public:

			valve_station::valve_view FliederErdbeeren() { return station.get_view(LUCAS_VALVE_1); }
			valve_station::valve_view Heidelbeeren() { return station.get_view(LUCAS_VALVE_2); }
			valve_station::valve_view Kartoffeln() { return station.get_view(LUCAS_VALVE_3); }


			inline void turn(bool on) const {
				return station.turn_all(on);
			}

			inline void turn_on() const {
				return turn(true);
			}

			inline void turn_off() const {
				return turn(false);
			}
		};


		felix_view Felix() { return felix_view(stations.at(0)); }
		james_view James() { return james_view(stations.at(1)); }
		lucas_view Lucas() { return lucas_view(stations.at(2)); }


		inline static landscape& instance() {
			if (!singleton_instance.has_value()) {
				singleton_instance = std::make_optional<landscape>(landscape());
			}
			return singleton_instance.value();
		}

	};

}

std::optional<k1::landscape> k1::landscape::singleton_instance;



void wait_for(int64_t duration_in_seconds) {
	auto start_time = time_helper::get_seconds_since_epoch_now();

	if constexpr (MANUAL_TEST) {
		duration_in_seconds /= 60;
		if (duration_in_seconds < 5) {
			duration_in_seconds = 5;
		}
	}

	while (time_helper::get_seconds_since_epoch_now() < start_time + duration_in_seconds) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void watering(const time_helper& start_time, k1::landscape& landscape) {

	//pumpe an
	send_mayson(0, 0, 0);

	std::this_thread::sleep_for(std::chrono::seconds(3));
	send_mayson(1, 1);
	std::this_thread::sleep_for(std::chrono::seconds(6));

	if (start_time.get_days_since_epoch() % 2 == 1) {

		landscape.James().Karotten().turn_on();
		landscape.Lucas().Kartoffeln().turn_on();

		wait_for(10 * 60);

		landscape.Felix().MaraSabine().turn_on();

		wait_for(10 * 60);

		send_mayson(0);
		wait_for(10);
		send_mayson(1);

		landscape.James().Karotten().turn_off();
		landscape.Felix().Eiben().turn_on();

		wait_for(10 * 60);

		landscape.Felix().MaraSabine().turn_off();

		wait_for(25 * 60);

		landscape.Felix().turn_off();
		landscape.James().turn_off();
		landscape.Lucas().turn_off();
	}

	send_mayson(0);
	wait_for(10);
	send_mayson(1);

	const bool BLAUBEER_TAG{ (start_time.get_days_since_epoch() % 4 == 0) };

	landscape.James().Gurken().turn_on();
	landscape.James().Tomaten().turn_on();
	landscape.Lucas().FliederErdbeeren().turn_on();
	if (BLAUBEER_TAG) landscape.Lucas().Heidelbeeren().turn_on();

	wait_for(20 * 60);

	landscape.Lucas().FliederErdbeeren().turn_off();
	landscape.James().Tomaten().turn_off();

	send_mayson(0);
	wait_for(10);
	send_mayson(1);

	wait_for(15 * 60);

	send_mayson(0);
	wait_for(10);
	send_mayson(1);

	wait_for(25 * 60);



	// END OF WATERING

	landscape.Felix().turn_off();
	landscape.James().turn_off();
	landscape.Lucas().turn_off();

	// wait for pressure reached
	wait_for(30);
	//pumpe aus
	send_mayson(0);


	// let capacitor run dry:
	landscape.Felix().Eiben().turn_on();
	wait_for(60 * 2);

	landscape.Felix().turn_off();
	landscape.James().turn_off();
	landscape.Lucas().turn_off();

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

	standard_logger()->info("Creating Landscape...");

	k1::landscape& garden = k1::landscape::instance();

	standard_logger()->info("Fetching timestamp...");
	const auto start_time = time_helper(true);

	const int64_t previous_timestamp = load_timestamp_file();

	const bool is_time_for_watering = check_if_in_watering_time_window(start_time, previous_timestamp);

	constexpr bool global_watering_enable{ true };

	if (MANUAL_TEST || (global_watering_enable && is_time_for_watering)) {
		watering(start_time, garden);
	}


	return 0;
}
