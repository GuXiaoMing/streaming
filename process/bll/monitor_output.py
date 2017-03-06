"""
# @file monitor_output.py
# @Synopsis  monitor the output of spark on local disk, and write the output to
# redis
# @author Ming Gu(guming@itv.baidu.com))
# @version 1.0
# @date 2016-10-28
"""
import sys
sys.path.append('..')
import os
from conf.env_config import EnvConfig

if __name__ == '__main__':
    data_path = os.path.join(EnvConfig.PROJECT_PATH, 'data', 'result')
    dirs = os.listdir(data_path)
    if len(dirs) > 0:
        dir_prefix = dirs[0].split('-')
        timestamps = map(lambda x: int(x.split('-')[1]) / 1000, dirs)
        print timestamps
