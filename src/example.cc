#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>

#include "RedisKVStore.h"

using namespace YiCppLib;

int main(int argc, char* argv[]) {

	RedisKVStore kvStore("/opt/redis/var/run/redis.sock");

	try{

		kvStore.setStringValueForKeyInNamespace("1", "0", "first");
		kvStore.setStringValueForKeyInNamespace("2", "0", "second");

		std::cout<<"value for 0 is "<<kvStore.stringValueForKeyInNamespace("0", "first")<<" in ns first"<<std::endl;
		std::cout<<"value for 0 is "<<kvStore.stringValueForKeyInNamespace("0", "second")<<" in ns second"<<std::endl;
		std::cout<<"value for 0 is "<<kvStore.stringValueForKeyInNamespace("0", "third")<<" in ns third"<<std::endl;



		kvStore.removeKeyInNamespace("0", "first");
		kvStore.removeKeyInNamespace("0", "second");


		kvStore.addStringValueToSetInNamespace("apple", "fruit");
		kvStore.addStringValueToSetInNamespace("pear", "fruit");
		kvStore.addStringValueToSetInNamespace("orange", "fruit");

		auto fruits = kvStore.stringSetValueForKeyInNamespace("fruit");
		std::cout<<"returned size: "<<fruits.size()<<std::endl;
		for(std::string& f : fruits) {
			std::cout<<"fruit: "<<f<<std::endl;
		}

		auto cars = kvStore.stringSetValueForKeyInNamespace("cars");
		std::cout<<"returned size: "<<cars.size()<<std::endl;
		for(std::string& c : cars) {
			std::cout<<"cars: "<<c<<std::endl;
		}

		kvStore.removeKeyInNamespace("fruit");
	}
	catch(const std::runtime_error& e) {
		std::cerr<<"exception: "<<e.what()<<std::endl;
	}


	return 0;
}
