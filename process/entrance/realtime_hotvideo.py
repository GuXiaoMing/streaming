"""
# @file realtime_hotvideo.py
# @Synopsis  calculate real time hot videos of each category
# @author Ming Gu(guming@itv.baidu.com))
# @version 1.0
# @date 2016-10-28
"""
import sys
sys.path.append('..')
from conf.env_config import EnvConfig
from conf.init_logger import initLogger
from dao.spark_submit import SparkSubmitter

if __name__ == '__main__':
    initLogger()
    spark_submitter = SparkSubmitter(EnvConfig.SPARK_CLIENT_PATH, EnvConfig.LOG_NAME)
    spark_submitter.submit_with_modules('../bll/hot_videos.py', '..', run_locally=True)
