#include <iostream>
#include <string>
#include <memory>

#include "RedisKVStore.h"

using namespace YiCppLib;

int main(int argc, char* argv[]) {

	RedisKVStore::pointer kvStore = RedisKVStore::redisKVStore("/opt/redis/var/run/redis.sock");

	kvStore->setStringValueForKey("1", "0");

	std::cout<<"value for 0 is "<<kvStore->stringValueForKey("0")<<std::endl;

	return 0;
}
