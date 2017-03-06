/**
 * @file body_processor.h
 * @Synopsis  process bin_body of received bigpipe message, which should be a
 * protocol buffer data, containing a 'all_data' field of json format. This
 * class get useful information from the json and return desirable format.
 * @author Ming Gu(guming@itv.baidu.com))
 * @version 1.0
 * @date 2016-09-05
 */

#ifndef MINOS_BIGPIPE_SUBSCRIBER_BODY_PROCESSOR_H
#define MINOS_BIGPIPE_SUBSCRIBER_BODY_PROCESSOR_H

#include "wise_video_webplayer.pb.h"
#include <json/json.h>
#include <exception>
#include <ctime>
#include "vector.h"
#include <map>
#include "uln_url.h"
#include "ul_sign.h"
#include <string>

namespace minos_bigpipe_subscriber
{
    class BodyProcessor
    {
        public:

            BodyProcessor();

            ~BodyProcessor();

            int process(const void* bin_body, size_t body_len, std::string &key,
                    std::string &member, double &score);

        private:

            std::vector<std::string> split(const std::string line, const std::string delimiter);

            std::map<std::string, std::string> get_query_dict(std::string query);

            std::string url_decode(const std::string &sSrc);

    };
}

#endif
