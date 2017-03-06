"""
# @file play_cnt.py
# @Synopsis  calculate the videos with most play cnts in the recent time window
# @author Ming Gu(guming@itv.baidu.com))
# @version 1.0
# @date 2016-10-21
"""
from __future__ import print_function

import sys
import os
import re
import json
from collections import Counter
import heapq
import logging

from conf.env_config import EnvConfig
from conf.init_logger import initLogger

from pyspark import SparkContext
from pyspark.streaming import StreamingContext


pattern = re.compile(r'[^\[]+\[ [^\]]* \]\[ [^\]]* \]\[ [^\]]* \]\[ [^\]]* \]\[ [^\]]* \]'
        '\[ [^\]]* \]\[ [^\]]* \]\[ [^\]]* \]\[ [^\]]* \]\[ ([^\]]*) \]')

def log_process(line):
    """
    # @Synopsis  process input log line
    # @Args line
    # @Returns   video as dict
    """
    ret = pattern.match(line)
    if ret is not None:
        info = ret.group(1).strip(' ')
        try:
            fields = info.split('\t')
            if 'wise_behavior_browse' in fields[0]:
                return None
            cuid = fields[0][len('wise_behavior_'):]
            video_json = json.loads(fields[1])
            title = video_json['title']
            album = video_json['album']
            sign1 = video_json['sign1']
            sign2 = video_json['sign2']
            link_sign = '{0},{1}'.format(sign1, sign2)
            album_fields = album.split('$$')
            if len(album_fields) == 2:
                category = album_fields[0]
            else:
                category = ''

            video = dict({
                'link_sign': link_sign,
                'title': title,
                'category': category,
                'album': album,
                })
            return cuid, video
        except Exception as e:
            print(e)

if __name__ == "__main__":
    initLogger()
    logger = logging.getLogger(EnvConfig.LOG_NAME)
    hostname = 'nj02-video-streaming.nj02.baidu.com'
    port = 9999

    sc = SparkContext(appName="CategoryHotVideos")
    ssc = StreamingContext(sc, 60)
    ssc.checkpoint('file:///home/video/guming02/tools/spark-scratch/checkpoint')
    ret_path = 'file://' + os.path.join(EnvConfig.PROJECT_PATH, 'data', 'result')

    lines = ssc.socketTextStream(hostname, port)

    video_playcnt = lines.map(log_process)\
            .filter(lambda x: x is not None)\
            .map(lambda x: ((x[1]['category'], x[1]['link_sign']), 1))\
            .reduceByKeyAndWindow(lambda x, y: x + y, lambda x, y: x - y, 3600, 60)

    # def rdd_mapper(rdd):
    #     category_histograms = rdd.collect()
    #     for category_histogram in category_histograms:
    #         category = category_histogram[0]
    #         histogram = category_histogram[1]
    #         print(u'{0}\t{1}'.format(category, histogram).encode('utf8'))

    category_hot_videos = video_playcnt\
            .map(lambda x: (x[0][0], (x[0][1], x[1])))\
            .groupByKey()\
            .map(lambda x: (x[0], filter(lambda a: a[1] > 0, x[1])))\
            .map(lambda x: (x[0], heapq.nlargest(10, x[1], key=lambda a: a[1])))\
            .map(lambda x: (u'wise_behavior_category_hotvideos_{0}'.format(x[0]), json.dumps(x[1])))\
            .map(lambda x: u'{0}\t{1}'.format(x[0], x[1]).encode('utf8'))\
            .saveAsTextFiles(os.path.join(ret_path, 'category_hot_videos'))



    # output_dstream = video_playcnt\
    #         .map(lambda x: (x[0], x[1].most_common(10)))\
    #         .map(print_mapper)
    # output_dstream.pprint()

    # output_dstream = video_playcnt\
    #         .transform(lambda rdd: rdd.sortBy(lambda x: x[1], ascending=False, numPartitions=1))\
    #         .map(lambda x: u'{0}\t{1}'.format(x[0], x[1]).encode('utf8'))

    # output_dstream.saveAsTextFiles('file:///home/video/guming02/test/play_cnt', 'txt')
    # output_dstream.pprint()


    # category_counts.foreachRDD(print_head)
            #.foreachRDD(lambda r: r.top(3, key=lambda x: -x[1]))
    # category_counts = lines.map(log_process)\
    #         .filter(lambda x: x is not None)\
    #         .map(lambda x: x[2])\
    #         .map(lambda x: (x.encode('utf8'), 1))\
    #         .updateStateByKey(updateFunc)



    ssc.start()
    ssc.awaitTermination()
