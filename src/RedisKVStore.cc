#include "RedisKVStore.h"
#include <hiredis/hiredis.h>

#include "log.h"

using namespace YiCppLib;

struct RedisKVStore::Impl {
	redisContext * rCtx;
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

void RedisKVStore::setStringValueForKey(const std::string& value, const std::string& key) {
	logger(LOGLV_INFO)<<"Setting "<<value<<" for key "<<key<<std::endl;
	auto reply = (redisReply*)redisCommand(pImpl_->rCtx, "SET key:%s %s", key.c_str(), value.c_str());
	logger(LOGLV_INFO)<<"Respond: "<<reply->type<<std::endl;
	freeReplyObject(reply);
}

std::string RedisKVStore::stringValueForKey(const std::string& key) {
	logger(LOGLV_INFO)<<"Retriving value for key "<<key<<std::endl;
	auto reply = (redisReply*)redisCommand(pImpl_->rCtx, "GET key:%s", key.c_str());
	logger(LOGLV_INFO)<<"Respond Type: "<<reply->type<<std::endl;
	std::string value(reply->str);
	logger(LOGLV_INFO)<<"Respond Value: "<<value<<std::endl;
	freeReplyObject(reply);
	return value;
}
