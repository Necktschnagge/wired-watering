
#include <string>
#include <iostream>

static const std::string IP_ADDRESS_PUMP_SERVER_MAYSON { "192.168.1.10" };
//static const std::string 

bool ping(const std::string& ip_address) {
	using std::cout, std::endl;

	std::string bash_command{ "ping -c1 -s1 " };
	bash_command.append(ip_address);
	bash_command.append("  > /dev/null 2>&1");
#ifdef LINUX
	int x = system(bash_command.c_str());
#else
	int x = 0;
#endif // LINUX
	if (x == 0) {
		cout << "success" << endl;
		return true;
	}
	else {
		cout << "failed" << endl;
		return false;
	}
}


int main(int argc, char** argv){

	(void)argc;
	(void)argv;



	ping(IP_ADDRESS_PUMP_SERVER_MAYSON);


	return 0;
}