/**
 * @file main.cpp
 * @Synopsis  main process
 * @author Ming Gu(guming@itv.baidu.com))
 * @version 1.0
 * @date 2016-09-06
 */

#include <pthread.h>
#include <signal.h>
#include <vector>
#include "Configure.h"
#include "gflags/gflags.h"
#include "data_processor.h"
#include "pipelet_subscriber.h"

namespace minos_bigpipe_subscriber
{
    // 主线程退出信号
    bool exiting = false;

    void signal_exit_handler(int arg)
    {
        BP_NOTICE("Signal %d catched", arg);
        exiting = true;
    }

    int app_main(int argc, char* argv[])
    {
        // 解析命令行参数conf_path和conf
        google::ParseCommandLineFlags(&argc, &argv, false);

        // 加载配置文件
        comcfg::Configure conf;

        if (conf.load("../conf", "video_webplayer.conf") != 0)
        {
            BP_FATAL("Failed to load conf file ../conf/video_webplayer.conf");
            return 1;
        }

        signal(SIGINT, signal_exit_handler);
        signal(SIGTERM, signal_exit_handler);

        DataProcessor* p_processor = new DataProcessor();
        int ret = p_processor->init();
        if (0 != ret)
        {
            BP_FATAL("Failed to init DataProcessor");
            return -1;
        }

        // 初始化pipelet订阅实例
        int pplt_num = conf["Bigpipe"]["pipelet_num"].to_int32();
        std::vector<PipeletSubscriber*> subscribers;

        for (int i = 0; i < pplt_num; ++i)
        {
            PipeletSubscriber* sub = new PipeletSubscriber();
            int ret = sub->init(
                        conf["Bigpipe"]["meta_host"].to_cstr(),
                        conf["Bigpipe"]["root_path"].to_cstr(),
                        conf["Bigpipe"]["log"].to_cstr(),
                        conf["Bigpipe"]["log_rotate_interval"].to_int32(),
                        conf["Account"]["acl_username"].to_cstr(),
                        conf["Account"]["acl_password"].to_cstr(),
                        conf["Bigpipe"]["pipe_name"].to_cstr(),
                        i + 1,
                        p_processor);

            if (0 != ret)
            {
                BP_WARN("Failed to initailize %s %d", conf["Bigpipe"]["pipe_name"].to_cstr(),
                        i + 1);
                delete sub;
                continue;
            }
            else
            {
                subscribers.push_back(sub);
            }
        }

        // 等待退出信号
        while (!exiting)
        {
            sleep(1);
        }

        for (int i = 0; i < pplt_num; ++i)
        {
            subscribers[i]->stop();
            delete subscribers[i];
            subscribers[i] = NULL;
        }

        delete p_processor;
        p_processor = NULL;

        BP_NOTICE("Exiting now");

        return 0;
    }
}

int main(int argc, char* argv[])
{
    return minos_bigpipe_subscriber::app_main(argc, argv);
}


