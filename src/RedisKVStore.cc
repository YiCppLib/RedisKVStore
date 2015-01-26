#include "RedisKVStore.h"
#include "hiredis.h"

#include "log.h"

using namespace YiCppLib;

#ifdef HAVE_CXX14
#define RETURN_REPLY_OBJECT(reply) return std::make_unique<RedisReply>(reply);
#else
#define RETURN_REPLY_OBJECT(reply) return reply_ptr(new RedisReply(reply));
#endif

#define KEY_WITH_NS(key, ns) ((ns) == "" ? (key) : (ns + ":" + key))

#ifndef LOGLVL
#define LOGLVL LOGLV_WARN
#endif

static auto logger = LOGGER(LOGLVL);
struct RedisKVStore::Impl {
	redisContext * rCtx;

	Impl(const std::string& ip, int port) : rCtx(nullptr) {
		logger(LOGLV_DEBUG)<<"creating RedisKVStore object [ip:"<<ip<<", port:"<<port<<"]"<<std::endl;

		rCtx = redisConnect(ip.c_str(), port);

		if(rCtx != NULL && rCtx->err) {
			logger(LOGLV_DEBUG)<<"Connection was not established, err: "<< rCtx->err<<std::endl;
		}

		logger(LOGLV_DEBUG)<<"RedisKVStore object created"<<std::endl;
	}

	Impl(const std::string& unixPath) : rCtx(nullptr) {
		logger(LOGLV_DEBUG)<<"creating RedisKVStore object [unix:"<<unixPath<<"]"<<std::endl;

		rCtx = redisConnectUnix(unixPath.c_str());

		if(rCtx != NULL && rCtx->err) {
			logger(LOGLV_DEBUG)<<"Connection was not established, err: "<< rCtx->err<<std::endl;
		}

		logger(LOGLV_DEBUG)<<"RedisKVStore object created"<<std::endl;
	}

	~Impl() {
		logger(LOGLV_DEBUG)<<"releasing RedisKVStore object"<<std::endl;
		if(rCtx != nullptr) redisFree(rCtx);
		logger(LOGLV_DEBUG)<<"RedisKVStore object released"<<std::endl;
	}
};

class RedisKVStore::RedisReply {
	private:
		redisReply * reply_;

	public:
		RedisReply() : reply_(nullptr) {}
		RedisReply(redisReply *reply) : reply_(reply) {}

		~RedisReply() {
			if(reply_) freeReplyObject(reply_);
		}

		std::string str() const noexcept { return std::string(reply_->str);}
		int type() const noexcept { return reply_->type;}
		size_t elements() const noexcept { return reply_->elements;}
		const redisReply * elementAt(size_t idx) const noexcept { return reply_->element[idx];}
};


RedisKVStore::RedisKVStore(const std::string& ip, int port) : pImpl_(new Impl(ip, port)) {
}

RedisKVStore::RedisKVStore(const std::string& unixPath) : pImpl_(new Impl(unixPath)) {
}

RedisKVStore::~RedisKVStore() = default;
RedisKVStore::RedisKVStore(RedisKVStore&& rhs) = default;
RedisKVStore& RedisKVStore::operator=(RedisKVStore&& rhs) = default;

RedisKVStore::pointer RedisKVStore::redisKVStore(const std::string& ip, int port) {
#ifdef HAVE_CXX14
	return std::make_shared<RedisKVStore>(ip, port);
#else
	return pointer(new RedisKVStore(ip, port));
#endif
}

RedisKVStore::pointer RedisKVStore::redisKVStore(const std::string& unixPath) {
#ifdef HAVE_CXX14
	return std::make_shared<RedisKVStore>(unixPath);
#else
	return pointer(new RedisKVStore(unixPath));
#endif
}

template<typename ... Types>
RedisKVStore::reply_ptr RedisKVStore::redisCommand(const std::string& cmd, const std::string& format, Types ... args) const noexcept {

	//logger(LOGLV_INFO)<<"EXEC: ["<<cmd<<"], ("<<format<<"), "<<args...<<std::endl;
	
	auto reply = (redisReply*)::redisCommand(pImpl_->rCtx, (cmd + " " + format).c_str(), args...);
	logger(LOGLV_INFO)<<"Respond: "<<reply->type<<std::endl;

	RETURN_REPLY_OBJECT(reply);
}

void RedisKVStore::removeKeyInNamespace(const std::string& key, const std::string& ns) const noexcept {
	redisCommand("DEL", "%s", KEY_WITH_NS(key, ns).c_str());
}

void RedisKVStore::setStringValueForKeyInNamespace(const std::string& value, const std::string& key, const std::string& ns) const noexcept {
	redisCommand("SET", "%s %s", KEY_WITH_NS(key, ns).c_str(), value.c_str());
}

std::string RedisKVStore::stringValueForKeyInNamespace(const std::string& key, const std::string& ns) const noexcept {
	return std::string(redisCommand("GET", "%s", KEY_WITH_NS(key, ns).c_str())->str());
}

/* ordered-set value operations */
void RedisKVStore::addStringValueToSetInNamespace(const std::string& value, const std::string& key, const std::string& ns) const noexcept {
	redisCommand("SADD", "%s %s", KEY_WITH_NS(key, ns).c_str(), value.c_str());
}

std::vector<std::string> RedisKVStore::stringSetValueForKeyInNamespace(const std::string& key, const std::string& ns) const noexcept{
	auto reply = redisCommand("SMEMBERS", "%s", KEY_WITH_NS(key, ns).c_str());
	std::vector<std::string> result;
	if(reply->type() != REDIS_REPLY_ARRAY) throw new std::runtime_error("Reply type is not array");
	logger(LOGLV_INFO)<<"returned array has a size of "<<reply->elements()<<std::endl;
	for(size_t i=0; i<reply->elements(); i++) {
		result.push_back(std::string(reply->elementAt(i)->str));
	}
	return result;
}
