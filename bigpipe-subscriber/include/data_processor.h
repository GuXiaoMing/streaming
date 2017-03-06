/**
 * @file data_processor.h
 * @Synopsis  process data received from bigpipe
 * @author Ming Gu(guming@itv.baidu.com))
 * @version 1.0
 * @date 2016-10-17
 */

#ifndef MINOS_BIGPIPE_SUBSCRIBER_DATA_PROCESSOR_H
#define MINOS_BIGPIPE_SUBSCRIBER_DATA_PROCESSOR_H

#include "redis_manager.h"

namespace minos_bigpipe_subscriber
{

class DataProcessor
{
public:

    DataProcessor();

    ~DataProcessor();

    int DataProcessor::init();

    int process(const void* bin_body, size_t body_len);

private:

    int save_in_file(const void* text, size_t text_len);

    int send_to_redis_task_queue(const void* bin_body, size_t body_len);

    RedisManager* _p_redis_manager;

};
}

#endif
