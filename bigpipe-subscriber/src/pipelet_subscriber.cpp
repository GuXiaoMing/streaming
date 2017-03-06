/**
 * @file pipelet_subscriber.cpp
 * @Synopsis  subscribe bigpipe piplet
 * @author Ming Gu(guming@itv.baidu.com))
 * @version 1.0
 * @date 2016-09-06
 */

#include "pipelet_subscriber.h"

namespace minos_bigpipe_subscriber
{
    PipeletSubscriber::PipeletSubscriber()
        : _meta(NULL),
        _pipelet(NULL),
        _meta_host(""),
        _root_path(""),
        _meta_log(""),
        _log_rotate_interval(1),
        _acl_username(""),
        _acl_password(""),
        _pipe_name(""),
        _pipelet_id(0),
        _p_data_processor(NULL)
    {
    }

    PipeletSubscriber::~PipeletSubscriber()
    {
        _pipelet->Close();
        delete _pipelet;
        _pipelet = NULL;

        delete _meta;
        _meta = NULL;
    }

    /*
    * 初始化meta, pipelet并启动订阅
    */
    int PipeletSubscriber::init(
        const char* meta_host,
        const char* root_path,
        const char* meta_log,
        int log_rotate_interval,
        const char* acl_username,
        const char* acl_password,
        const char* pipe_name,
        int pipelet_id,
        DataProcessor* data_processor)
    {
        _meta_host = std::string(meta_host);
        _root_path = std::string(root_path);
        _meta_log = std::string(meta_log);
        _log_rotate_interval = log_rotate_interval;
        _acl_username = std::string(acl_username);
        _acl_password = std::string(acl_password);
        _pipe_name = std::string(pipe_name);
        _pipelet_id = pipelet_id;
        _p_data_processor = data_processor;

        return init_pipelet();
    }

    /*
    * 关闭pipelet, 停止订阅
    */
    void PipeletSubscriber::stop()
    {
        _pipelet->Close();
    }

    /*
    * 初始化pipelet实例并启动订阅。如果已经初始化过，就析构掉再重新初始化
    */
    int PipeletSubscriber::init_pipelet()
    {
        if (NULL != _pipelet)
        {
            _pipelet->Close();
            delete _pipelet;
        }

        // 会出core
        /*if (NULL != _meta)
        {
            delete _meta;
        }*/

        // 连接到meta_host
        _meta = new bigpipe::api::Meta(_meta_host.c_str(), _root_path.c_str());
        bigpipe::api::Meta::Option opt;
        opt.log_file = _meta_log.c_str();
        opt.log_rotate_interval = _log_rotate_interval;
        _meta->SetOption(opt);

        int ret = _meta->Connect();

        if (ret != bigpipe::api::kOk)
        {
            BP_FATAL("Failed to connect. meta_host=%s, root_path=%s, ret=%d",
                    _meta_host.c_str(), _root_path.c_str(), ret);
            return -1;
        }
        else
        {
            BP_NOTICE("Successfully connected. meta_host=%s, root_path=%s",
                    _meta_host.c_str(), _root_path.c_str());
        }

        _pipelet = new bigpipe::api::Pipelet(
            _meta,
            _acl_username.c_str(),
            _acl_password.c_str(),
            _pipe_name.c_str(),
            _pipelet_id);

        if (NULL == _pipelet)
        {
            BP_FATAL("Failed to create pipelet instance for %s %d", _pipe_name.c_str(),
                    _pipelet_id);
            return -1;
        }

        _pipelet->SetCallback(on_msg, this, on_error, this);

        // 启动订阅
        bigpipe::api::Pipelet::Option pplt_opt;
        pplt_opt.preference = 0.5;
        pplt_opt.retry_limit = 10;
        pplt_opt.retry_interval = 1;

        // 订阅点传-1，从最新的数据开始接收
        // 传-2的话从最老的数据开始接收
        if (_pipelet->Subscribe(-1, &pplt_opt) != bigpipe::api::kOk)
        {
            BP_WARN("Failed to subscribe to pipelet %s %d", _pipe_name.c_str(), _pipelet_id);
            return -1;
        }
        else
        {
            BP_NOTICE("Successfully subscribed to pipelet %s %d", _pipe_name.c_str(), _pipelet_id);
        }

        return 0;
    }

    /*
    * 消息处理函数
    */
    void PipeletSubscriber::on_msg(bigpipe::api::Pipelet* pplt, const bigpipe::api::Message& msg,
                                void* ctx)
    {
        (void)pplt;

        if (NULL == ctx)
        {
            BP_FATAL("ctx pointer is NULL");
            return;
        }

        BP_NOTICE("new message %"PRIu64" of %s, size = %zu",
                msg.id(), msg.topic().c_str(), msg.size());

        // 取出入参
        PipeletSubscriber* p_subscriber = reinterpret_cast<PipeletSubscriber*>(ctx);

        std::vector<bigpipe::api::Message::Fragment> fragments;
        msg.GetFragments(&fragments);

        for (uint32_t i = 0; i < fragments.size(); ++i)
        {
            const void* bin_body;
            size_t body_len = 0;

            try
            {
                compack::Buffer bufwrap((void*)fragments[i].data, fragments[i].size);
                compack::buffer::Reader reader(bufwrap);
                compack::ObjectIterator element = compack::ObjectIterator();

                if (!reader.find("bin_body", element))
                {
                    BP_WARN("Failed to get bin_body field");
                }
                else
                {
                    bin_body = element.getAsBinary();
                    body_len = element.length();
                    int ret = p_subscriber->_p_data_processor->process(bin_body, body_len);
                    if (0 != ret)
                    {
                        BP_WARN("Failed to process data");
                    }
                }
            }
            catch (std::exception& ex)
            {
                BP_WARN("Exception thrown when parsing fragment: %s", ex.what());
                continue;
            }
            catch (...)
            {
                BP_WARN("Failed to parse the fragment, skipping it");
                continue;
            }

        }
    }

    /*
    * 错误处理函数
    */
    void PipeletSubscriber::on_error(bigpipe::api::Pipelet* pplt, int err_no, void* ctx)
    {
        (void)pplt;
        PipeletSubscriber* p_subscriber = reinterpret_cast<PipeletSubscriber*>(ctx);

        BP_WARN("pipelet %s %d errno=%d, will re-initialize pipelet",
                p_subscriber->_pipe_name.c_str(), p_subscriber->_pipelet_id, err_no);

        p_subscriber->init_pipelet();
    }
}


