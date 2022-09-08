#pragma once

#include <string>
#include <exception>

#ifndef GIT_REPOSITORY_URL
#error C++ macro GIT_REPOSITORY_URL is undefined. Please set it in your CMakeLists.txt using something like ""add_compile_definitions(GIT_REPOSITORY_URL="https://github.com/user-name/repository-name")"". Notice: Place this line before project declaration.
#endif // GIT_REPOSITORY_URL

namespace fsl {
	class internal_error : public std::exception {
		std::string message;

		inline static const std::string INTERNAL_ERROR{ "Internal Error:  " };
		inline static const std::string PLEASE_REPORT{ std::string("  Please report this to the developers: Open an issue at ") + GIT_REPOSITORY_URL + "/issues and copy-paste the error message. If possible add a description how to reproduce this error." };

	public:
		static void assert_true(bool condition, const internal_error& exception) {
			if (!condition) throw exception;
		}

		internal_error(const std::string& message) : message(INTERNAL_ERROR + message + PLEASE_REPORT) {}
		internal_error(const char* message) : internal_error(std::string(message)) {}
		internal_error(const internal_error&) = default;

		const char* what() const noexcept override {
			return message.c_str();
		}
	};
//### TODO put this class into some fsl repository so that it can be reused
}