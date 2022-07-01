#include "logger.h"

#include "internal_error.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/ostream_sink.h"

#include <iostream>


namespace {
	
	const std::string STANDARD_LOGGER_NAME{ "main" };

}


std::shared_ptr<spdlog::logger> standard_logger(){
	auto s_ptr = spdlog::get(STANDARD_LOGGER_NAME);
	fsl::internal_error::assert_true(s_ptr.operator bool(), "The standard logger has not been registered.");
	return s_ptr;
	// when returning raw pointer a warning is logged since the shared_ptr will be destroyed, but the underlying pointer will be retained
}

void init_logger(){
	auto sink_std_cout = std::make_shared<spdlog::sinks::ostream_sink_mt>(std::cout);

	auto standard_logger = std::make_shared<spdlog::logger>(STANDARD_LOGGER_NAME, sink_std_cout);

	standard_logger->set_level(spdlog::level::trace);

	spdlog::register_logger(standard_logger);
}


