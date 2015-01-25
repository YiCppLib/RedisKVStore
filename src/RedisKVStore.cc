#include "RedisKVStore.h"
#include <hiredis/hiredis.h>

#include "log.h"

using namespace YiCppLib;

struct RedisKVStore::Impl {
	redisContext * rCtx;
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

class ConcreteRedisKVStore : public RedisKVStore {
	public:
		ConcreteRedisKVStore(const std::string&ip, int port) 
			: RedisKVStore(ip, port)
		{}

		ConcreteRedisKVStore(const std::string& unixPath)
			: RedisKVStore(unixPath)
		{}
};

static auto logger = LOGGER(LOGLV_DEBUG);

RedisKVStore::RedisKVStore(const std::string& ip, int port) :
	pImpl_(new Impl()) 
{
	logger(LOGLV_DEBUG)<<"creating RedisKVStore object [ip:"<<ip<<", port:"<<port<<"]"<<std::endl;

	pImpl_->rCtx = redisConnect(ip.c_str(), port);

	if(pImpl_->rCtx != NULL && pImpl_->rCtx->err) {
		logger(LOGLV_DEBUG)<<"Connection was not established, err: "<< pImpl_->rCtx->err<<std::endl;
	}

	logger(LOGLV_DEBUG)<<"RedisKVStore object created"<<std::endl;

}

RedisKVStore::RedisKVStore(const std::string& unixPath) :
	pImpl_(new Impl()) 
{
	
	logger(LOGLV_DEBUG)<<"creating RedisKVStore object [unix:"<<unixPath<<"]"<<std::endl;

	pImpl_->rCtx = redisConnectUnix(unixPath.c_str());

	if(pImpl_->rCtx != NULL && pImpl_->rCtx->err) {
		logger(LOGLV_DEBUG)<<"Connection was not established, err: "<< pImpl_->rCtx->err<<std::endl;
	}

	logger(LOGLV_DEBUG)<<"RedisKVStore object created"<<std::endl;
}

RedisKVStore::~RedisKVStore() {
	logger(LOGLV_DEBUG)<<"releasing RedisKVStore object"<<std::endl;
	redisFree(pImpl_->rCtx);
	logger(LOGLV_DEBUG)<<"RedisKVStore object released"<<std::endl;
	
}

RedisKVStore::RedisKVStore(RedisKVStore&& rhs) = default;
RedisKVStore& RedisKVStore::operator=(RedisKVStore&& rhs) = default;

RedisKVStore::pointer RedisKVStore::redisKVStore(const std::string& ip, int port) {
#ifdef HAVE_CXX14
	return std::make_shared<ConcreteRedisKVStore>(ip, port);
#else
	return pointer(new ConcreteRedisKVStore(ip, port));
#endif
}

RedisKVStore::pointer RedisKVStore::redisKVStore(const std::string& unixPath) {
#ifdef HAVE_CXX14
	return std::make_shared<ConcreteRedisKVStore>(unixPath);
#else
	return pointer(new ConcreteRedisKVStore(unixPath));
#endif
}

RedisKVStore::reply_ptr RedisKVStore::redisCommand_1arg(const std::string& cmd, const std::string& format, const std::string& arg) const {
	logger(LOGLV_INFO)<<"EXEC: ["<<cmd<<"], ("<<format<<"), "<<arg<<std::endl;
	auto reply = (redisReply*)redisCommand(pImpl_->rCtx, (cmd + " " + format).c_str(), arg.c_str());
	logger(LOGLV_INFO)<<"Respond: "<<reply->type<<std::endl;

#ifdef HAVE_CXX14
	return std::make_unique<RedisReply>(reply);
#else
	return reply_ptr(new RedisReply(reply));
#endif
}


RedisKVStore::reply_ptr RedisKVStore::redisCommand_2arg(const std::string& cmd, const std::string& format, const std::string& arg1, const std::string& arg2) {
	logger(LOGLV_INFO)<<"EXEC: ["<<cmd<<"], ("<<format<<"), "<<arg1<<", "<<arg2<<std::endl;
	auto reply = (redisReply*)redisCommand(pImpl_->rCtx, (cmd + " " + format).c_str(), arg1.c_str(), arg2.c_str());
	logger(LOGLV_INFO)<<"Respond: "<<reply->type<<std::endl;

#ifdef HAVE_CXX14
	return std::make_unique<RedisReply>(reply);
#else
	return reply_ptr(new RedisReply(reply));
#endif
}

std::string RedisKVStore::stringValueForKey(const std::string& key) const noexcept{
	return std::string(redisCommand_1arg("GET", "%s", key)->str());
}

void RedisKVStore::setStringValueForKey(const std::string& value, const std::string& key) {
	redisCommand_2arg("SET", "%s %s", key, value);
}

void RedisKVStore::setStringValueForKeyInNamespace(const std::string& value, const std::string& key, const std::string& ns) noexcept {
	setStringValueForKey(value, ns != "" ? ns + ":" + key : key);
}

std::string RedisKVStore::stringValueForKeyInNamespace(const std::string& key, const std::string& ns) const noexcept{
	return stringValueForKey(ns != "" ? ns + ":" + key : key);
}

/* ordered-set value operations */
void RedisKVStore::addStringValueToSet(const std::string& value, const std::string& key) {
	redisCommand_2arg("SADD", "%s %s", key, value);
}

void RedisKVStore::addStringValueToSetInNamespace(const std::string& value, const std::string& key, const std::string& ns) {
	addStringValueToSet(value, ns != "" ? ns + ":" + key : key);
}

std::vector<std::string> RedisKVStore::stringSetValueForKey(const std::string& key) const{
	auto reply = redisCommand_1arg("SMEMBERS", "%s", key);
	std::vector<std::string> result;
	if(reply->type() != REDIS_REPLY_ARRAY) throw new std::runtime_error("Reply type is not array");
	logger(LOGLV_INFO)<<"returned array has a size of "<<reply->elements()<<std::endl;
	for(size_t i=0; i<reply->elements(); i++) {
		result.push_back(std::string(reply->elementAt(i)->str));
	}
	return result;
}

std::vector<std::string> RedisKVStore::stringSetValueForKeyInNamespace(const std::string& key, const std::string& ns) const{
	return stringSetValueForKey(ns != "" ? ns + ":" + key : key);
}
