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

			static pointer redisKVStore(const std::string& ip, int port);
			static pointer redisKVStore(const std::string& unixPath);

			/* string value operations */
			std::string stringValueForKey(const std::string& key) const noexcept;
			void setStringValueForKey(const std::string& value, const std::string& key);
			void setStringValueForKeyInNamespace(const std::string& value, const std::string& key, const std::string& ns) noexcept;
			std::string stringValueForKeyInNamespace(const std::string& key, const std::string& ns) const noexcept;

			/* ordered-set value operations */
			void addStringValueToSet(const std::string& value, const std::string& key);
			void addStringValueToSetInNamespace(const std::string& value, const std::string& key, const std::string& ns);
			std::vector<std::string> stringSetValueForKey(const std::string& key) const;
			
		protected:
			RedisKVStore(const std::string& ip, int port);
			RedisKVStore(const std::string& unixPath);

			reply_ptr redisCommand_1arg(const std::string& cmd, const std::string& format, const std::string& arg) const;
			reply_ptr redisCommand_2arg(const std::string& cmd, const std::string& format, const std::string& arg1, const std::string& arg2);

	};
}

#endif
