/**
 * @file redis_manager.cpp
 * @Synopsis  unpack bin body and write to redis
 * @author Ming Gu(guming@itv.baidu.com))
 * @version 1.0
 * @date 2016-09-05
 */

#include <errno.h>
#include "redis_manager.h"
#include "body_processor.h"
#include "Configure.h"

namespace minos_bigpipe_subscriber
{
    RedisManager::RedisManager() :
        _p_cli_mgr(NULL)
    {
        _task_queue_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    }

    RedisManager::~RedisManager()
    {
        void* param = NULL;

        for (int i = 0; i < _writer_thread_num; ++i)
        {
            int ret = pthread_join(_pid[i], &param);

            if (0 != ret)
            {
                UB_LOG_FATAL("Failed to join redis writer thread in redis manager, errno=%d",
                        errno);
            }
        }
    }

    int RedisManager::init()
    {
        int ret = 0;
        // init log
        ret = ub_log_init("../log", "videoredis.", 100, UL_LOG_NOTICE);
        if (ret != 0) {
            fprintf(stderr, "failed to init ub_log\n");
            return -1;
        }

        _p_cli_mgr = new store::RedisClientManager();
        ret = _p_cli_mgr->init("../conf", "redisdb.conf");

        if (ret != 0)
        {
            fprintf(stderr, "RedisClientManager init failed\n");
            return -1;
        }

        comcfg::Configure conf;
        if (conf.load("../conf", "video_webplayer.conf") != 0)
        {
            BP_FATAL("Failed to load conf file ../conf/video_webplayer.conf");
            return -1;
        }

        _writer_thread_num = conf["Redis"]["write_thread_num"].to_int32();
        _expire_sec = (long int)conf["Redis"]["expire_sec"].to_int32();
        _pid = new pthread_t[_writer_thread_num];

        for (int i = 0; i < _writer_thread_num; ++i)
        {
            int ret = pthread_create(&_pid[i], NULL, writer_thread_func, this);
            if (0 != ret)
            {
                BP_FATAL("Failed to create thread");
                return -1;
            }
        }
        return 0;
    }

    void RedisManager::push_task(const void* data, size_t len)
    {
        bin_body_t bin_body;
        bin_body.data = malloc(len);
        memcpy(bin_body.data, data, len);
        bin_body.len = len;
        pthread_mutex_lock(&_task_queue_mutex);
        _task_queue.push(bin_body);
        pthread_mutex_unlock(&_task_queue_mutex);
    }

    void* RedisManager::writer_thread_func(void* ptr)
    {
        ub_log_initthread("thread");

        RedisManager* redis_manager = (RedisManager*)ptr;

        store::RedisClientManager *cli_mgr = redis_manager->_p_cli_mgr;
        store::RedisClient *client = NULL;

        if (!cli_mgr)
        {
            UB_LOG_FATAL("cli_mgr is NULL");
            ub_log_closethread();
            return NULL;
        }

        client = cli_mgr->create_client();
        if (!client) {
            UB_LOG_FATAL("cli_mgr->create_client failed");
            ub_log_closethread();
            return NULL;
        }

        BodyProcessor body_processor;

        while (true)
        {
            pthread_mutex_lock(&(redis_manager->_task_queue_mutex));
            if (redis_manager->_task_queue.empty())
            {
                pthread_mutex_unlock(&(redis_manager->_task_queue_mutex));
                usleep(50); //in microsecond
                continue;
            }

            bin_body_t bin_body = redis_manager->_task_queue.front();
            redis_manager->_task_queue.pop();
            pthread_mutex_unlock(&(redis_manager->_task_queue_mutex));

            std::string key;
            std::string member;
            double score = 0.0;

            //将二进制数据进行解析，解析出要写入redis的数据结构, 即写入sorted
            //set 数据结构的key, member, score
            int ret = body_processor.process(bin_body.data, bin_body.len,
                    key, member, score);

            if (0 == ret)
            {
                int result = 0;
                store::SimpleString *member_list = new store::SimpleString(member.c_str());
                const double *score_list = new double(score);
                //把key, member, score写入redis的sorted set 数据结构
                ret = client->zadd(key.c_str(), member_list, score_list, 1, result);
                if (ret != 0)
                {
                    UB_LOG_WARNING("zadd failed, ret:%d, err_no:%d, err_msg:%s",
                        ret, client->get_err_no(), client->get_err_msg());
                }
                else
                {
                    UB_LOG_NOTICE("%s\t%s\t%d", key.c_str(), member.c_str(), (int)score);
                }

                time_t t = std::time(0);
                long int now = static_cast<long int> (t);
                long int max_time = now - redis_manager->_expire_sec;
                long int min_time = -1;
                //删除一个key下对应的sorted set里的过期数据，sorted
                //set里是每个member对应一个value, 就是该member的时间戳，如果该时间戳在
                //[-1, 当前时间-失效时长]这个范围内，就把该member 删除
                ret = client->zrem_range_by_score(key.c_str(), (double)min_time,
                        (double)max_time, result);
                if (ret != 0)
                {
                    UB_LOG_WARNING("zrem_range_by_score failed, ret:%d, err_no:%d, err_msg:%s",
                        ret, client->get_err_no(), client->get_err_msg());
                }
                //更新当前写入的key的失效时长，刷新为配置中指定的时长
                ret = client->expire(key.c_str(), (int)redis_manager->_expire_sec, result);
                if (ret != 0)
                {
                    UB_LOG_WARNING("expire failed, ret:%d, err_no:%d, err_msg:%s",
                        ret, client->get_err_no(), client->get_err_msg());
                }
            }

            delete bin_body.data;
        }
        return NULL;
    }
}

