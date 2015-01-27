#include "RedisKVStore.h"
#include "hiredis.h"
#include <cstdio>
#include <stdexcept>
#include <sstream>

#include "log.h"

using namespace YiCppLib;

#ifdef HAVE_CXX14
#define REPLY_UPTR(reply) std::make_unique<RedisReply>(reply)
#else
#define REPLY_UPTR(reply) std::unique_ptr<RedisReply>(new RedisReply(reply))
#endif

#define KEY_WITH_NS(key, ns) ((ns) == "" ? (key) : (ns + ":" + key))
#define KEYNS_CSTR(key, ns) KEY_WITH_NS(key, ns).c_str()

#define CHECK_REPLY_STATUS(reply, expected) \
{ \
	if((reply).get() == nullptr) {\
		std::stringstream errMsg; \
		errMsg<<"Reply status error in "<<__func__<<", command returned nil, err: "<<pImpl_->err(); \
		throw std::runtime_error(errMsg.str()); \
	} \
	else if((reply)->type() != (expected)) { \
		std::stringstream errMsg; \
		errMsg<<"Reply status error in "<<__func__<<", expecting "<<(expected)<<"; got "<<reply->type(); \
		throw std::runtime_error(errMsg.str()); \
	} \
}

#ifndef LOGLVL
#define LOGLVL LOGLV_WARN
#endif

static auto logger = LOGGER(LOGLVL);

class RedisKVStore::RedisReply {
	private:
		redisReply * reply_;
		const bool standalone_;

	public:
		RedisReply() : RedisReply(nullptr, false) {}
		RedisReply(redisReply *reply, bool standalone = true) : reply_(reply), standalone_(standalone) {}

		~RedisReply() {
			if(reply_ && standalone_) freeReplyObject(reply_);
		}

		std::string str() const noexcept { return std::move<std::string>(std::string(reply_->str));}
		int type() const noexcept { return reply_->type;}
		size_t elements() const noexcept { return reply_->elements;}
		RedisReply elementAt(size_t idx) const noexcept { return std::move<RedisReply>(RedisReply(reply_->element[idx], false));}
};

struct RedisKVStore::Impl {
	private:
		redisContext * rCtx;

	public:

		Impl(const std::string& ip, int port) : rCtx(nullptr) {
			logger(LOGLV_DEBUG)<<"creating RedisKVStore object [ip:"<<ip<<", port:"<<port<<"]"<<std::endl;

			rCtx = redisConnect(ip.c_str(), port);

			if(rCtx != NULL && rCtx->err) {
				logger(LOGLV_ERR)<<"Connection was not established, err: "<< rCtx->err<<std::endl;
				throw std::runtime_error("Unable to connect to database");
			}

			logger(LOGLV_DEBUG)<<"RedisKVStore object created"<<std::endl;
		}

		Impl(const std::string& unixPath) : rCtx(nullptr) {
			logger(LOGLV_DEBUG)<<"creating RedisKVStore object [unix:"<<unixPath<<"]"<<std::endl;

			rCtx = redisConnectUnix(unixPath.c_str());

			if(rCtx != NULL && rCtx->err) {
				logger(LOGLV_ERR)<<"Connection was not established, err: "<< rCtx->err<<std::endl;
				throw std::runtime_error("Unable to connect to database");
			}

			logger(LOGLV_DEBUG)<<"RedisKVStore object created"<<std::endl;
		}

		~Impl() {
			logger(LOGLV_DEBUG)<<"releasing RedisKVStore object"<<std::endl;
			if(rCtx != nullptr) redisFree(rCtx);
			logger(LOGLV_DEBUG)<<"RedisKVStore object released"<<std::endl;
		}

		auto err() -> decltype(rCtx->err) const {
			return rCtx->err;
		}

		template<class ... Args>
		RedisKVStore::reply_ptr redisCommand(const std::string& cmd, const std::string& format, Args&&... args) const {
			char *argStr;
			asprintf(&argStr, (cmd + " " +format).c_str(), std::forward<Args>(args)...);
			logger(LOGLV_INFO)<<"Redis-Exec: "<<argStr<<std::endl;
			free(argStr);

			auto reply = (redisReply*)::redisCommand(rCtx, (cmd + " " + format).c_str(), std::forward<Args>(args)...);
			if(reply == NULL) return REPLY_UPTR(nullptr);
			return REPLY_UPTR(reply);
		}
};

RedisKVStore::RedisKVStore(const std::string& ip, int port) : pImpl_(new Impl(ip, port)) {
}

RedisKVStore::RedisKVStore(const std::string& unixPath) : pImpl_(new Impl(unixPath)) {
}

RedisKVStore::~RedisKVStore() = default;
RedisKVStore::RedisKVStore(RedisKVStore&& rhs) = default;
RedisKVStore& RedisKVStore::operator=(RedisKVStore&& rhs) = default;

void RedisKVStore::removeKeyInNamespace(const std::string& key, const std::string& ns) const {
	auto reply = pImpl_->redisCommand("DEL", "%s", KEYNS_CSTR(key, ns));
	CHECK_REPLY_STATUS(reply, REDIS_REPLY_INTEGER);
}

void RedisKVStore::setStringValueForKeyInNamespace(const std::string& value, const std::string& key, const std::string& ns) const {
	auto reply = pImpl_->redisCommand("SET", "%s %s", KEYNS_CSTR(key, ns), value.c_str());
	CHECK_REPLY_STATUS(reply, REDIS_REPLY_STATUS);
}

std::string RedisKVStore::stringValueForKeyInNamespace(const std::string& key, const std::string& ns) const {
	auto reply = pImpl_->redisCommand("GET", "%s", KEYNS_CSTR(key, ns));
	if(reply->type() == REDIS_REPLY_NIL)
		return "";

	CHECK_REPLY_STATUS(reply, REDIS_REPLY_STRING);
	return std::string(reply->str());
}

/* ordered-set value operations */
void RedisKVStore::addStringValueToSetInNamespace(const std::string& value, const std::string& key, const std::string& ns) const {
	auto reply = pImpl_->redisCommand("SADD", "%s %s", KEYNS_CSTR(key, ns), value.c_str());
	CHECK_REPLY_STATUS(reply, REDIS_REPLY_INTEGER);
}

std::vector<std::string> RedisKVStore::stringSetValueForKeyInNamespace(const std::string& key, const std::string& ns) const {
	auto reply = pImpl_->redisCommand("SMEMBERS", "%s", KEYNS_CSTR(key, ns));

	if(reply->type() == REDIS_REPLY_NIL)
		return std::vector<std::string>();

	std::vector<std::string> result;
	CHECK_REPLY_STATUS(reply, REDIS_REPLY_ARRAY);

	logger(LOGLV_INFO)<<"returned array has a size of "<<reply->elements()<<std::endl;
	for(size_t i=0; i<reply->elements(); i++) {
		result.push_back(std::string(reply->elementAt(i).str()));
	}
	return result;
}
