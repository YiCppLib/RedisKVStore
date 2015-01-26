#ifndef YICPPLIB_REDISKVSTORE_H
#define YICPPLIB_REDISKVSTORE_H

#include <memory>
#include <string>
#include <vector>

namespace YiCppLib {

	class RedisKVStore {
		private:
			struct Impl;
			std::unique_ptr<Impl> pImpl_;
			class RedisReply;


		public:
			using pointer = std::shared_ptr<RedisKVStore>;
			using reply_ptr = std::unique_ptr<RedisReply>;

			virtual ~RedisKVStore();						// dtor
			RedisKVStore(RedisKVStore&& rhs);				// move ctor
			RedisKVStore& operator=(RedisKVStore&& rhs);	// move ctor

			RedisKVStore(const std::string& ip, int port);
			RedisKVStore(const std::string& unixPath);

			/* remove key */
			void removeKeyInNamespace(const std::string& key, const std::string& ns = "") const ;

			/* string value operations */
			void setStringValueForKeyInNamespace(const std::string& value, const std::string& key, const std::string& ns = "") const ;
			std::string stringValueForKeyInNamespace(const std::string& key, const std::string& ns = "") const ;

			/* ordered-set value operations */
			void addStringValueToSetInNamespace(const std::string& value, const std::string& key, const std::string& ns = "")const ;
			std::vector<std::string> stringSetValueForKeyInNamespace(const std::string& key, const std::string& ns = "") const ;

			
		private:
			
			template<typename ... Types>
			reply_ptr redisCommand(const std::string& cmd, const std::string& format, Types ... args) const;

	};
}

#endif
