/**
 * @file body_processor.cpp
 * @Synopsis  process bin_body
 * @author Ming Gu(guming@itv.baidu.com))
 * @version 1.0
 * @date 2016-09-05
 */
#include "redis_manager.h"
#include "body_processor.h"

namespace minos_bigpipe_subscriber
{
    BodyProcessor::BodyProcessor()
    {
    }

    BodyProcessor::~BodyProcessor()
    {
    }

    int BodyProcessor::process(const void* bin_body, size_t body_len, std::string &key,
                    std::string &member, double &score)
    {
        int ret = -1;
        //protocol buffer的解析类
        baidu::video::WiseVideoWebplayer msg;
        //protocol buffer解码
        if (!msg.ParseFromArray(bin_body, body_len))
        {
            UB_LOG_WARNING("failed to parse protocol buffer bin_body: %s", (const char*)bin_body);
            return -1;
        }

        try{
            std::string msg_type = msg.type();
            std::string msg_flag = msg.flag();
            if (0 == msg_type.compare("nsclick")){
                std::string data = msg.all_data();
                //log里的数据是一个url的形式，把前面的补全成为完整的url，前面加什么无所谓，
                //只是后续使用的url处理函数需要输入是一个合法的url字符串
                std::string url = "http://v.baidu.com" + data;

                uln_set_encode_mode(ULN_UN_ESCAPED_MODE);
                //uln_set_encode_mode(ENCODE_ESCAPED_MODE);
                uln_url_t url_t;
                if (0 != uln_parse_url(url.c_str(), &url_t)) {
                    UB_LOG_WARNING("failed to parse url: %s", url.c_str());
                    return -1;
                }
                //解析url，取出url中的参数
                uln_normalize_url(&url_t);
                //path 就是/v.gif或/p.gif
                char path[100];
                if (uln_get_url_segment(&url_t, SEG_URL_PATH, path, sizeof(path)) != 0) {
                    UB_LOG_WARNING("failed to get path from url: %s", url.c_str());
                    return -1;
                }
                //取出url问号后面的参数部分
                char buf[1000];
                if (uln_get_url_segment(&url_t, SEG_URL_QUERY, buf, sizeof(buf)) != 0) {
                    UB_LOG_WARNING("failed to get query from url: %s", url.c_str());
                    return -1;
                }

                //std::cout << data << std::endl;
                std::string query = std::string(buf);
                //把url中的参数解析成k v的map形式
                std::map<std::string, std::string> query_dict = get_query_dict(query);

                std::string channel_name = query_dict["channel_name"];
                std::string pid = query_dict["pid"];
                std::string time = query_dict["cur_time"].substr(0, 10);
                std::string cuid = query_dict["cuid"];
                //the cuid for iphone has the prefix: 02:00:00:00:00:00_, we should trim it.
                if (0 == cuid.find("02:00:00:00:00:00_", 0)) {
                    cuid = cuid.replace(0, 18, "");
                }

                //这是志祥给出的app个性化推荐区块的日志打log的标识字段，用于过滤出该区块的log
                if (0 == channel_name.compare("channel_shortrecommand") &&
                        (0 == pid.compare("185") || 0 == pid.compare("186"))) {
                    //这是浏览日志打的log
                    if (0 == std::string(path).compare("/p.gif")) {
                        std::cout << data << std::endl;
                        std::string signs = query_dict["signs"];
                        key = "wise_behavior_browse_" + cuid;
                        member = signs;
                        score = (double)atoi(time.c_str());
                        return 0;
                    }
                    //这是播放日志打的log
                    else if (0 == std::string(path).compare("/v.gif")) {
                        std::string album = query_dict["album"];
                        std::string video_name = query_dict["video_name"];
                        std::string u = query_dict["u"];

                        Json::Value root_out;
                        Json::FastWriter writer;

                        root_out["title"] = video_name;
                        root_out["album"] = album;
                        root_out["time"] = time;
                        std::string url = u;
                        //root_out["url"] = url;

                        char* buf = new char[url.length() + 1];
                        snprintf(buf, url.length() + 1, "%s", url.c_str());

                        unsigned int sign1 = 0;
                        unsigned int sign2 = 0;
                        if (1 != creat_sign_fs64(buf, strlen(buf), &sign1, &sign2)){
                            UB_LOG_WARNING("Failed to create link sign");
                            return -1;
                        }

                        delete [] buf;

                        root_out["sign1"] = sign1;
                        root_out["sign2"] = sign2;

                        std::string json_out = writer.write(root_out);
                        //erase the tailing newline
                        json_out.erase(json_out.end() - 1);

                        key = "wise_behavior_" + cuid;
                        member = json_out;
                        score = (double)atoi(time.c_str());
                        return 0;
                    }
                }
            }
        }
        catch (std::exception& e){
                UB_LOG_WARNING("Failed to get all_data from bin_body: %s", (const char*)bin_body);
                return -1;
        }
        return ret;
    }

    std::vector<std::string> BodyProcessor::split(const std::string line,
            const std::string delimiter) {
        vector<std::string> parts;
        std::string part;
        size_t pos = 0;
        std::string remainer = std::string(line);
        while ((pos = remainer.find(delimiter)) != std::string::npos) {
            part = remainer.substr(0, pos);
            parts.push_back(part);
            remainer.erase(0, pos + delimiter.length());
        }
        parts.push_back(remainer);
        return parts;
    }

    std::map<std::string, std::string> BodyProcessor::get_query_dict(std::string query){
        std::map<std::string, std::string> query_dict;
        std::map<std::string, std::string>::iterator it;
        std::vector<std::string> parts = split(query, "&");
        for (int i = 0; i < parts.size(); i++){
            std::vector<std::string> kv = split(parts[i], "=");
            if (2 == kv.size()) {
                it = query_dict.find(kv[0]);
                if (it == query_dict.end()) {
                    //这里要自己decode，厂里的url解析类很弱，没法处理urlencode之后的数据
                    query_dict[kv[0]] = url_decode(kv[1]);
                }
            }
        }
        return query_dict;
    }

    std::string BodyProcessor::url_decode(const std::string & sSrc)
    {
        // Note from RFC1630: "Sequences which start with a percent
        // sign but are not followed by two hexadecimal characters
        // (0-9, A-F) are reserved for future extension"

        const char HEX2DEC[256] = 
        {
            /*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
            /* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
            /* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
            /* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
            /* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,
            
            /* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
            /* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
            /* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
            /* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
            
            /* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
            /* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
            /* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
            /* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
            
            /* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
            /* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
            /* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
            /* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
        };
        const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
        const int SRC_LEN = sSrc.length();
        const unsigned char * const SRC_END = pSrc + SRC_LEN;
        // last decodable '%'
        const unsigned char * const SRC_LAST_DEC = SRC_END - 2;

        char * const pStart = new char[SRC_LEN];
        char * pEnd = pStart;

        while (pSrc < SRC_LAST_DEC)
        {
            if (*pSrc == '%')
            {
                char dec1 = '0';
                char dec2 = '0';
                if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
                    && -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
                {
                    *pEnd++ = (dec1 << 4) + dec2;
                    pSrc += 3;
                    continue;
                }
            }

            *pEnd++ = *pSrc++;
        }

        // the last 2- chars
        while (pSrc < SRC_END) {
            *pEnd++ = *pSrc++;
        }

        std::string sResult(pStart, pEnd);
        delete [] pStart;
        return sResult;
    }

}
