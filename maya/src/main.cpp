

#include "cpr/cpr.h"

#include <string>
#include <iostream>
#include <chrono>

static const std::string IP_ADDRESS_PUMP_SERVER_MAYSON { "192.168.1.10" };
static const std::string IP_ADDRESS_VALVE_SERVER_JAMES { "192.168.1.20" };
static const std::string IP_ADDRESS_VALVE_SERVER_LUCAS { "192.168.1.21" };
static const std::string IP_ADDRESS_VALVE_SERVER_FELIX { "192.168.1.22" };
static const std::string TEST_ADRESS_PING_FAIL { "192.168.2.233" };
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


int main(int argc, char** argv){

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

	
	auto time_since_0 = std::chrono::system_clock::now().time_since_epoch();
	
	std::cout << time_since_0.count() << std::endl;
	std::cout << time_since_0.count()/10000000 << std::endl;
	std::cout << (time_since_0.count()/600000000) % (24*60)<< std::endl; // hour of the day in UTC (we are 2 hours ahead.)

	return 0;
}