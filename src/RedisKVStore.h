#ifndef YICPPLIB_REDISKVSTORE_H
#define YICPPLIB_REDISKVSTORE_H

#include <memory>
#include <string>

namespace YiCppLib {
	class RedisKVStore {
		public:
			using pointer = std::shared_ptr<RedisKVStore>;

			virtual ~RedisKVStore();						// dtor
			RedisKVStore(RedisKVStore&& rhs);				// move ctor
			RedisKVStore& operator=(RedisKVStore&& rhs);	// move ctor

			static pointer redisKVStore(const std::string& ip, int port);
			static pointer redisKVStore(const std::string& unixPath);

			void setStringValueForKey(const std::string& value, const std::string& key);
			std::string stringValueForKey(const std::string& key);

			
		protected:
			RedisKVStore(const std::string& ip, int port);
			RedisKVStore(const std::string& unixPath);

		private:
			struct Impl;
			std::unique_ptr<Impl> pImpl_;
	};
}

#endif
