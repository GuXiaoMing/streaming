/**
 * @file pipelet_subscriber.h
 * @Synopsis  subscribe bigpipe pipelet
 * @author Ming Gu(guming@itv.baidu.com))
 * @version 1.0
 * @date 2016-10-17
 */

#ifndef MINOS_BIGPIPE_SUBSCRIBER_PIPELET_SUBSCRIBER_H
#define MINOS_BIGPIPE_SUBSCRIBER_PIPELET_SUBSCRIBER_H

#include "bigpipe.h"
#include "compack/compack.h"
#include "data_processor.h"

namespace minos_bigpipe_subscriber
{
class PipeletSubscriber
{
public:
    PipeletSubscriber();

    ~PipeletSubscriber();

    int init(
        const char* meta_host,
        const char* root_path,
        const char* meta_log,
        int log_rotate_interval,
        const char* acl_username,
        const char* acl_password,
        const char* pipe_name,
        int pipelet_id,
        DataProcessor* processor);

    void stop();

private:

    int init_pipelet();

    static void on_msg(bigpipe::api::Pipelet* pplt, const bigpipe::api::Message& msg, void* ctx);

    static void on_error(bigpipe::api::Pipelet* pplt, int err_no, void* ctx);

    bigpipe::api::Meta* _meta;

    bigpipe::api::Pipelet::Pipelet* _pipelet;

    std::string _meta_host;

    std::string _root_path;

    std::string _meta_log;

    int _log_rotate_interval;

    std::string _acl_username;

    std::string _acl_password;

    std::string _pipe_name;

    int _pipelet_id;

    DataProcessor* _p_data_processor;
};

}

#endif

