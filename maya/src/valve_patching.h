#pragma once


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

		[[maybe_unused]] inline static const std::string LUCAS_VALVE_1_LABEL{ "Bohnen-Flieder" };
		[[maybe_unused]] inline static const std::string LUCAS_VALVE_2_LABEL{ "Heidelbeeren" };
		[[maybe_unused]] inline static const std::string LUCAS_VALVE_3_LABEL{ "Karotten" };

		[[maybe_unused]] static constexpr uint8_t LUCAS_BAER_BOHNEN_UND_FLIEDER{ RAW_VALVES::LUCAS_VALVE_1 };
		[[maybe_unused]] static constexpr uint8_t LUCAS_HEIDELBEEREN{ RAW_VALVES::LUCAS_VALVE_2 };
		[[maybe_unused]] static constexpr uint8_t LUCAS_KAROTTEN{ RAW_VALVES::LUCAS_VALVE_3 };

	}

}


