/**
 * @file redis_manager.h
 * @Synopsis  manage redis queue
 * @author Ming Gu(guming@itv.baidu.com))
 * @version 1.0
 * @date 2016-10-17
 */

#ifndef MINOS_BIGPIPE_SUBSCRIBER_REDIS_MANAGER_H
#define MINOS_BIGPIPE_SUBSCRIBER_REDIS_MANAGER_H

#include <pthread.h>
#include <queue>
#include <string>
#include "bigpipe.h"
#include "body_processor.h"

//MAX_ADDR_LEN is defined as macro in a linux header file which is included in
//the code, however, in somewhere RedisiClientManager.h includes, that is a
//variable name, which cause trouble. Since the macro is actually not used
//anywhere(grep the whole project), the most convinient way is to just undefine
//it.
#ifdef MAX_ADDR_LEN
#undef MAX_ADDR_LEN
#endif
#include "RedisClientManager.h"

namespace minos_bigpipe_subscriber
{
    struct bin_body_t
    {
        void* data;
        size_t len;
    };

    class RedisManager
    {
    public:
        RedisManager();

        ~RedisManager();

        //int init(const char* redis_ip, int redis_port, const char* list_name);
        int init();

        // 要写入redis的字符串放入任务队列
        void push_task(const void* bin_body, size_t body_len);

    private:
        store::RedisClientManager* _p_cli_mgr;
        // redis写线程主函数
        static void* writer_thread_func(void* arg);

        // 写redis的任务队列
        std::queue<bin_body_t> _task_queue;

        // redis写线程访问任务队列时的互斥信号量
        pthread_mutex_t _task_queue_mutex;

        // redis写线程数量
        int _writer_thread_num;

        // redis写线程
        pthread_t *_pid;

        int _expire_sec;

    };
}

#endif

