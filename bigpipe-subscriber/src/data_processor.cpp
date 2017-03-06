/**
 * @file data_processor.cpp
 * @Synopsis  process data received from bigpipe
 * @author Ming Gu(guming@itv.baidu.com))
 * @version 1.0
 * @date 2016-09-06
 */

#include "bigpipe.h"
#include "data_processor.h"

namespace minos_bigpipe_subscriber
{
    DataProcessor::DataProcessor() :
        _p_redis_manager(NULL)

    {
    }

    DataProcessor::~DataProcessor()
    {
        if (NULL != _p_redis_manager)
        {
            delete _p_redis_manager;
            _p_redis_manager = NULL;
        }
    }

    int DataProcessor::init()
    {
        _p_redis_manager = new RedisManager();
        int ret = _p_redis_manager->init();

        if (0 != ret) {
            BP_FATAL("Failed to initialize redis manager");
            delete _p_redis_manager;
            _p_redis_manager = NULL;
            return -1;
        }
        return 0;
    }

    int DataProcessor::process(const void* bin_body, size_t body_len)
    {
        int ret = send_to_redis_task_queue(bin_body, body_len);
        return ret;
    }

    int DataProcessor::send_to_redis_task_queue(const void* bin_body, size_t body_len)
    {
        if (NULL == _p_redis_manager)
        {
            BP_FATAL("Pointer to redis manager is NULL");
            return -1;
        }

        //将bin_body入队，等待处理
        _p_redis_manager->push_task(bin_body, body_len);
        return 0;
    }
}
