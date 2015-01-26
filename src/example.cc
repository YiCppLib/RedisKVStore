#include <iostream>
#include <string>
#include <memory>

#include "RedisKVStore.h"

using namespace YiCppLib;

int main(int argc, char* argv[]) {

	RedisKVStore::pointer kvStore = RedisKVStore::redisKVStore("/opt/redis/var/run/redis.sock");

	kvStore->setStringValueForKeyInNamespace("1", "0", "first");
	kvStore->setStringValueForKeyInNamespace("2", "0", "second");

	std::cout<<"value for 0 is "<<kvStore->stringValueForKeyInNamespace("0", "first")<<" in ns first"<<std::endl;
	std::cout<<"value for 0 is "<<kvStore->stringValueForKeyInNamespace("0", "second")<<" in ns second"<<std::endl;

	kvStore->removeKeyInNamespace("0", "first");
	kvStore->removeKeyInNamespace("0", "second");


	kvStore->addStringValueToSetInNamespace("apple", "fruit");
	kvStore->addStringValueToSetInNamespace("pear", "fruit");
	kvStore->addStringValueToSetInNamespace("orange", "fruit");

	auto fruits = kvStore->stringSetValueForKeyInNamespace("fruit");
	std::cout<<"returned size: "<<fruits.size()<<std::endl;
	for(std::string& f : fruits) {
		std::cout<<"fruit: "<<f<<std::endl;
	}

	kvStore->removeKeyInNamespace("fruit");

	return 0;
}
