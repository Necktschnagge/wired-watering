
#include "logger.h"
#include "configs.h"

#include "cpr/cpr.h"
#include <nlohmann/json.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <chrono>

#include <fstream>
#include <optional>

#include <sstream>
#include <iomanip>

static constexpr bool MANUAL_TEST{ false };

namespace CONF {

	namespace IP_ADDRESS {
		[[maybe_unused]] static const std::string PUMP_SERVER_MAYSON{ "192.168.1.10" };

		[[maybe_unused]] static const std::string VALVE_SERVER_JAMES{ "192.168.1.20" };
		[[maybe_unused]] static const std::string VALVE_SERVER_LUCAS{ "192.168.1.21" };
		[[maybe_unused]] static const std::string VALVE_SERVER_FELIX{ "192.168.1.22" };

		[[maybe_unused]] static const std::string VALVE_SERVER_TEST{ "192.168.1.23" };

		[[maybe_unused]] static const std::string TEST_PING_FAIL{ "192.168.2.233" };
	}

	// server names / device names:
	[[maybe_unused]] inline static const std::string FELIX{ "Felix" };
	[[maybe_unused]] inline static const std::string JAMES{ "James" };
	[[maybe_unused]] inline static const std::string LUCAS{ "Lucas" };
	[[maybe_unused]] inline static const std::string MAYSON{ "Mayson" };

	namespace RAW_VALVES {

		[[maybe_unused]] static constexpr uint8_t JAMES_VALVE_1{ 0b00000001 };
		[[maybe_unused]] static constexpr uint8_t JAMES_VALVE_2{ 0b00000010 };
		[[maybe_unused]] static constexpr uint8_t JAMES_VALVE_3{ 0b00000100 };
		[[maybe_unused]] static constexpr uint8_t JAMES_VALVE_4{ 0b00001000 };

		[[maybe_unused]] static constexpr uint8_t LUCAS_VALVE_1{ 0b00000001 };
		[[maybe_unused]] static constexpr uint8_t LUCAS_VALVE_2{ 0b00000010 };
		[[maybe_unused]] static constexpr uint8_t LUCAS_VALVE_3{ 0b00000100 };

		[[maybe_unused]] static constexpr uint8_t FELIX_VALVE_1{ 0b00000001 };
		[[maybe_unused]] static constexpr uint8_t FELIX_VALVE_2{ 0b00000010 };

	}
	namespace VALVE_MAP {

		// valve patch infos:
		[[maybe_unused]] inline static const std::string FELIX_VALVE_1_LABEL{ "Klee Feld" };
		[[maybe_unused]] inline static const std::string FELIX_VALVE_2_LABEL{ "Eiben Klee" };

		[[maybe_unused]] static constexpr uint8_t FELIX_KLEE{ RAW_VALVES::FELIX_VALVE_1 };
		[[maybe_unused]] static constexpr uint8_t FELIX_EIBEN{ RAW_VALVES::FELIX_VALVE_2 };

		[[maybe_unused]] inline static const std::string JAMES_VALVE_1_LABEL{ "Erbsen" };
		[[maybe_unused]] inline static const std::string JAMES_VALVE_2_LABEL{ "Kartoffeln" };
		[[maybe_unused]] inline static const std::string JAMES_VALVE_3_LABEL{ "Gurken" };
		[[maybe_unused]] inline static const std::string JAMES_VALVE_4_LABEL{ "Tomaten+Mara" };

		[[maybe_unused]] static constexpr uint8_t JAMES_ERBSEN{ RAW_VALVES::JAMES_VALVE_1 };
		[[maybe_unused]] static constexpr uint8_t JAMES_KARTOFFEL{ RAW_VALVES::JAMES_VALVE_2 };
		[[maybe_unused]] static constexpr uint8_t JAMES_GURKEN{ RAW_VALVES::JAMES_VALVE_3 };
		[[maybe_unused]] static constexpr uint8_t JAMES_TOMATE_MARA_SABINE{ RAW_VALVES::JAMES_VALVE_4 };

		[[maybe_unused]] inline static const std::string LUCAS_VALVE_1_LABEL{ "Karotten" };
		[[maybe_unused]] inline static const std::string LUCAS_VALVE_2_LABEL{ "Heidelbeeren" };
		[[maybe_unused]] inline static const std::string LUCAS_VALVE_3_LABEL{ "Bohnen-Flieder" };

		[[maybe_unused]] static constexpr uint8_t LUCAS_1____{ RAW_VALVES::LUCAS_VALVE_1 };
		[[maybe_unused]] static constexpr uint8_t LUCAS_HEIDELBEEREN{ RAW_VALVES::LUCAS_VALVE_2 };
		[[maybe_unused]] static constexpr uint8_t LUCAS_BAER_BOHNEN_UND_FLIEDER{ RAW_VALVES::LUCAS_VALVE_3 };

	}

}





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
	url += CONF::IP_ADDRESS::PUMP_SERVER_MAYSON;
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

		std::string get_duration_table() const {
			std::string table;
			for (std::size_t i = 0; i < valves.size(); ++i) {
				std::string padded_station_label = label;
				padded_station_label.insert(padded_station_label.end(), 8 - padded_station_label.size(), ' ');
				std::string padded_valve_label = valves[i];
				padded_valve_label.insert(padded_valve_label.begin(), 20 - padded_valve_label.size(), ' ');

				const auto duration_minutes = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<60, 1>>>(accumulated_ms[i]).count();
				std::stringstream duration_minutes_ss;
				duration_minutes_ss << std::fixed << std::setprecision(2) << duration_minutes;
				std::string padded_duration{ duration_minutes_ss.str() };
				padded_duration.insert(padded_duration.begin(), 10 - padded_duration.size(), ' ');

				table += padded_station_label + ":" + padded_valve_label + "  :" + padded_duration + " min" + "\n";
			}
			return table;
		}

		void send(bool enable_log = true) {
			send_valves(ip_address, valves_state, enable_log);
		}

		void turn(bool on, uint8_t valve_bit_mask) {
			standard_logger()->info("\n--------------------------------------------------------------");
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
				"\nSwitched the following valves: --->\n"
			};

			for (uint8_t i = 0; i < valves.size(); ++i) {
				const uint8_t mask{ static_cast<uint8_t>(uint8_t(1) << i) };
				const bool switched = (old_state & mask) != (valves_state & mask);
				const std::string next_string = (valves_state & mask) ? "ON " : "OFF";
				const std::string prev_string = (valves_state & mask) ? "OFF" : " ON";
				if (switched)
					message += "          " + label + ":   " + valves.at(i) + "   " + prev_string + " -> " + next_string + "\n";
			}
			message +=
				"                                   <---";

			standard_logger()->info(message);

			last_switching_time = now;
		}

		inline uint8_t mask_all() {
			uint8_t mask{ 0 };
			for (std::size_t i = 0; i < valves.size(); ++i) {
				mask |= uint8_t(1) << i;
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
			stations.push_back(valve_station(CONF::FELIX, CONF::IP_ADDRESS::VALVE_SERVER_FELIX, std::vector<std::string>{ CONF::VALVE_MAP::FELIX_VALVE_1_LABEL, CONF::VALVE_MAP::FELIX_VALVE_2_LABEL}));

			standard_logger()->info("Creating James...");
			stations.push_back(valve_station(CONF::JAMES, CONF::IP_ADDRESS::VALVE_SERVER_JAMES, std::vector<std::string>{ CONF::VALVE_MAP::JAMES_VALVE_1_LABEL, CONF::VALVE_MAP::JAMES_VALVE_2_LABEL, CONF::VALVE_MAP::JAMES_VALVE_3_LABEL, CONF::VALVE_MAP::JAMES_VALVE_4_LABEL}));

			standard_logger()->info("Creating Lucas...");
			stations.push_back(valve_station(CONF::LUCAS, CONF::IP_ADDRESS::VALVE_SERVER_LUCAS, std::vector<std::string>{ CONF::VALVE_MAP::LUCAS_VALVE_1_LABEL, CONF::VALVE_MAP::LUCAS_VALVE_2_LABEL, CONF::VALVE_MAP::LUCAS_VALVE_3_LABEL}));
		}

	public:

		class felix_view {
			valve_station& station;

			friend class landscape;

			felix_view(valve_station& _station) : station(_station) {}

		public:

			valve_station::valve_view Klee2024() { return station.get_view(CONF::VALVE_MAP::FELIX_KLEE); }
			valve_station::valve_view Eiben2024() { return station.get_view(CONF::VALVE_MAP::FELIX_EIBEN); }

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

			valve_station::valve_view Erbsen2024() { return station.get_view(CONF::VALVE_MAP::JAMES_ERBSEN); }
			valve_station::valve_view Gurken2024() { return station.get_view(CONF::VALVE_MAP::JAMES_GURKEN); }
			valve_station::valve_view Kartoffel2024() { return station.get_view(CONF::VALVE_MAP::JAMES_KARTOFFEL); }
			valve_station::valve_view TomateMaraSabine2024() { return station.get_view(CONF::VALVE_MAP::JAMES_TOMATE_MARA_SABINE); }

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

			valve_station::valve_view Karotten2024() { return station.get_view(CONF::VALVE_MAP::LUCAS_1____); }
			valve_station::valve_view Heidelbeeren2024() { return station.get_view(CONF::VALVE_MAP::LUCAS_HEIDELBEEREN); }
			valve_station::valve_view BohnenFlieder2024() { return station.get_view(CONF::VALVE_MAP::LUCAS_BAER_BOHNEN_UND_FLIEDER); }


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

		std::string get_duration_table() const {
			std::string table;
			for (std::size_t i = 0; i < stations.size(); ++i) {
				table += stations[i].get_duration_table();
			}
			return table;
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
	(void)start_time;

	const auto all_valves_off{
		[&]() {
		landscape.James().turn_off();
		landscape.Lucas().turn_off();
		landscape.Felix().turn_off();
		std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	};

	//pumpe an
	send_mayson(0, 0, 0);

	auto drain_valve{ landscape.Lucas().Heidelbeeren2024() };

	drain_valve.turn_on();
	std::this_thread::sleep_for(std::chrono::seconds(30));
	send_mayson(1, 1);
	std::this_thread::sleep_for(std::chrono::seconds(60));

	all_valves_off();

	if (start_time.get_days_since_epoch() % 2) {

		landscape.James().Erbsen2024().turn_on(); // 30min
		landscape.James().Gurken2024().turn_on(); // 30min
		landscape.James().TomateMaraSabine2024().turn_on(); // 30min

		landscape.Lucas().Heidelbeeren2024().turn_on(); // 30min
		landscape.Lucas().Karotten2024().turn_on(); // 30min

		wait_for(30 * 60);

		all_valves_off();

		landscape.James().Erbsen2024().turn_on(); // 50min
		landscape.James().Gurken2024().turn_on(); // 50min

		landscape.Lucas().Karotten2024().turn_on(); // 50min

		wait_for(20 * 60);

		all_valves_off();

		landscape.James().Gurken2024().turn_on(); // 90min
		landscape.Felix().Eiben2024().turn_on(); // 40min

		wait_for(40 * 60);

		all_valves_off();

		//landscape.Felix().Klee2024().turn_on(); // 20min
		landscape.Lucas().Heidelbeeren2024().turn_on(); // 50min

		wait_for(20 * 60);
	}
	else {

		landscape.James().Erbsen2024().turn_on(); // 30min
		landscape.James().TomateMaraSabine2024().turn_on(); // 30min
		landscape.James().Kartoffel2024().turn_on(); // 30min

		landscape.Lucas().BohnenFlieder2024().turn_on(); // 30min

		wait_for(30 * 60);

		all_valves_off();

		landscape.Lucas().Heidelbeeren2024().turn_on(); // 30min
		landscape.Lucas().BohnenFlieder2024().turn_on(); // 60min

		wait_for(15 * 60);

		landscape.James().Erbsen2024().turn_on(); // 45min
		landscape.James().Gurken2024().turn_on(); // 15min

		wait_for(15 * 60);

		all_valves_off();

		landscape.Felix().Eiben2024().turn_on(); // 30min
		landscape.James().Gurken2024().turn_on(); // 45min

		wait_for(30 * 60);
	}

	// END OF WATERING

	landscape.Felix().turn_off();
	landscape.James().turn_off();
	landscape.Lucas().turn_off();

	// wait for pressure reached
	wait_for(30);
	//pumpe aus
	send_mayson(0);


	// let capacitor run dry:
	//landscape.Felix().Klee2024().turn_on();
	drain_valve.turn_on();
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

class ping_checker {

	static bool check_all_servers_using_ping_once() {

		standard_logger()->info("Pinging Pump Server Mayson...");

		const bool MAYSON_AVAILABLE{ ping(CONF::IP_ADDRESS::PUMP_SERVER_MAYSON) };
		if (!MAYSON_AVAILABLE) {
			standard_logger()->error("Fatal: Pump Server not available!");
		}
		else {
			standard_logger()->info("Pump Server ping OK!");
		}

		standard_logger()->info("Pinging Pump Server Mayson   ...DONE!");

		standard_logger()->info("Pinging all valve stations...");
		const bool JAMES_AVAILABLE{ ping(CONF::IP_ADDRESS::VALVE_SERVER_JAMES) };
		const bool LUCAS_AVAILABLE{ ping(CONF::IP_ADDRESS::VALVE_SERVER_LUCAS) };
		const bool FELIX_AVAILABLE{ ping(CONF::IP_ADDRESS::VALVE_SERVER_FELIX) };

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

		const bool TEST_SERVER_UNAVAILABLE{ !ping(CONF::IP_ADDRESS::VALVE_SERVER_TEST) };

		if (!TEST_SERVER_UNAVAILABLE) {
			standard_logger()->error("Got successful PING from Device that should not exists on local network!");
		}
		else {
			standard_logger()->info("Pinging Non-Existing Test Server OK!");
		}

		standard_logger()->info("Pinging Non-Existing Test Server   ...DONE!");

		return MAYSON_AVAILABLE && JAMES_AVAILABLE && LUCAS_AVAILABLE && FELIX_AVAILABLE && TEST_SERVER_UNAVAILABLE;
	}

public:
	static bool check_ping_devices() {
		for (std::size_t i{ 0 }; i < 20; ++i) {
			const bool PING_OK{ check_all_servers_using_ping_once() };
			if (PING_OK)
				return true;
		}
		return false;
	}

};

class telegram_interface {

	std::string bot_secret;

	std::string get_base_url() const noexcept {
		return std::string("https://api.telegram.org/bot") + bot_secret;
	}
	class endpoints {
		friend class telegram_interface;
		inline static const std::string getMe{ "/getMe" };
		inline static const std::string getUpdates{ "/getUpdates" };
		inline static const std::string getChat{ "/getChat" };
		inline static const std::string sendMessage{ "/sendMessage" };
	};
	class keys {
		friend class telegram_interface;
		inline static const std::string chat_id{ "chat_id" };
		inline static const std::string disable_notification{ "disable_notification" };
		inline static const std::string parse_mode{ "parse_mode" };
		inline static const std::string text{ "text" };
	};

	class exceptions {
	public:

		template<int ERROR_CODE>
		class unexpected_response_status_code : public std::runtime_error {
		public:
			unexpected_response_status_code(int actual_status_code) : std::runtime_error(
				std::string("Got HTTP response code ") + std::to_string(actual_status_code) + ". Expected status code was " + std::to_string(ERROR_CODE) + "."
			) {}
		};
	};

public:
	telegram_interface(const std::string& bot_secret) : bot_secret(bot_secret) {

	}

	telegram_interface(const telegram_interface&) = default;
	telegram_interface(telegram_interface&&) = default;

	telegram_interface& operator = (const telegram_interface&) = default;
	telegram_interface& operator = (telegram_interface&&) = default;

	nlohmann::json getMe() {
		auto params = cpr::Parameters();
		cpr::Response r = cpr::Get(
			cpr::Url{ get_base_url() + endpoints::getMe },
			params
		);
		if (r.status_code != 200) {
			throw exceptions::unexpected_response_status_code<200>(r.status_code);
		}
		return nlohmann::json::parse(r.text);
	}

	nlohmann::json getUpdates() {
		auto params = cpr::Parameters();
		cpr::Response r = cpr::Get(
			cpr::Url{ get_base_url() + endpoints::getUpdates },
			params
		);
		if (r.status_code != 200) {
			throw exceptions::unexpected_response_status_code<200>(r.status_code);
		}
		return nlohmann::json::parse(r.text);
	}

	nlohmann::json getChat(long long chat_id) {
		auto params = cpr::Parameters();
		params.Add(cpr::Parameter(keys::chat_id, std::to_string(chat_id)));
		cpr::Response r = cpr::Get(
			cpr::Url{ get_base_url() + endpoints::getChat },
			params
		);
		if (r.status_code != 200) {
			throw exceptions::unexpected_response_status_code<200>(r.status_code);
		}
		return nlohmann::json::parse(r.text);
	}

	nlohmann::json sendMessage(long long chat_id, const std::string& text, bool disable_notification = false, const std::string& parse_mode = "MarkdownV2") {
		auto params = cpr::Parameters();
		params.Add(cpr::Parameter(keys::chat_id, std::to_string(chat_id)));
		params.Add(cpr::Parameter(keys::text, text));
		params.Add(cpr::Parameter(keys::disable_notification, disable_notification ? "true" : "false"));
		params.Add(cpr::Parameter(keys::parse_mode, parse_mode));
		cpr::Response r = cpr::Get(
			cpr::Url{ get_base_url() + endpoints::sendMessage },
			params
		);
		if (r.status_code != 200) {
			throw exceptions::unexpected_response_status_code<200>(r.status_code);
		}
		return nlohmann::json::parse(r.text); // check parse error
	}

};

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
		(start_time.get_minute_intra_day() > (5 - 2) * 60 + 1) // 5:01 // -2 == UTC 
		&& (start_time.get_minute_intra_day() < (12 - 2) * 60 + 1) // 12:01
		&& (previous_timestamp + 10 * 60 + 1 < start_time.get_minutes_since_epoch()) // 10 hours gone since last watering
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

std::optional<maya::telegram_config> load_telegram_config() {

	if (!std::filesystem::exists(maya::config::PATH_TO_TELEGRAM_JSON)) {
		standard_logger()->error("Telegram secret config does not exist!");
		standard_logger()->error(
			//std::filesystem::canonical(
			std::filesystem::absolute(
				std::filesystem::path(
					maya::config::PATH_TO_TELEGRAM_JSON
				)
			)
			//)
			.string());
		return std::optional<maya::telegram_config>();
	}

	auto telegram_secret_istream = std::ifstream(maya::config::PATH_TO_TELEGRAM_JSON);
	auto telegram_json = nlohmann::json::parse(std::istream_iterator<char>(telegram_secret_istream), std::istream_iterator<char>()); // check for errors!
	return std::make_optional<maya::telegram_config>(telegram_json);
}

int main(int argc, char** argv) {

	(void)argc;
	(void)argv;

	init_logger();

	// load telegram secrets:

	std::optional<maya::telegram_config> tel_config = load_telegram_config();

	std::optional<telegram_interface> tel;

	if (tel_config) {
		tel.emplace(tel_config.value().bot_secret);
	}

	const bool devices_available = ping_checker::check_ping_devices();

	if (!devices_available) {
		try {
			if (tel) tel.value().sendMessage(tel_config.value().main_chat_id, "Devices ping error!");
		}
		catch (...) {
		}
	}

	standard_logger()->info("Creating Landscape...");

	k1::landscape& garden = k1::landscape::instance();

	standard_logger()->info("Fetching timestamp...");
	const auto start_time = time_helper(true);

	const int64_t previous_timestamp = load_timestamp_file();

	const bool is_time_for_watering = check_if_in_watering_time_window(start_time, previous_timestamp);

	constexpr bool global_watering_enable{ false };

	const bool START_WATERING{ MANUAL_TEST || (global_watering_enable && is_time_for_watering) };

	if (START_WATERING) {
		try {
			if (tel) tel.value().sendMessage(tel_config.value().main_chat_id, "Start watering now\\!");
		}
		catch (...)
		{
		}
		watering(start_time, garden);
	}

	standard_logger()->info(std::string("Accumulated watering times:\n\n") + garden.get_duration_table());

	if (tel
		&& START_WATERING
		) {
		try {
			std::string message{ "Finished watering now\\!\n\n```\n" };
			message += garden.get_duration_table();
			message += "```";
			tel.value().sendMessage(tel_config.value().main_chat_id, message);
		}
		catch (...) {
		}
	}

	return 0;
}
