"""
##
# @file env_config.py
# @Synopsis  config environment
# @author Ming Gu(guming02@baidu.com))
# @version 1.0
# @date 2015-09-06
"""
import os
import datetime as dt

class EnvConfig(object):
    """
    # @Synopsis  config environment
    """
    PROJECT_NAME = 'streaming-process'
    LOG_NAME = PROJECT_NAME
    DEBUG = True
    MAIL_RECEIVERS = ['guming@itv.baidu.com']

    #find project root path according to the path of this config file
    CONF_PATH = os.path.split(os.path.realpath(__file__))[0]
    # if the 'conf' module is provided to spark-submit script in a .zip file,
    # the real path of this file would be project_path/conf.zip/conf(refer to
    # the dal.spark_submit module), while the
    # real path of config file we wanna locate is project_path/conf, thus the
    # following transformation would be neccessary.
    if '.zip' in CONF_PATH:
        path_stack = CONF_PATH.split('/')
        CONF_PATH = '/'.join(path_stack[:-2]) + '/conf'
    PROJECT_PATH = os.path.join(CONF_PATH, '../')
    LOG_PATH = os.path.join(PROJECT_PATH, 'log')
    GENERAL_LOG_FILE = os.path.join(LOG_PATH, 'general.log')
    #script path

    #tool path
    if DEBUG:
        TOOL_PATH = '/home/video/guming02/tools/'
    else:
        TOOL_PATH = '/home/video/guming/tools'

    HADOOP_CLIENT_PATH = os.path.join(TOOL_PATH, 'hadoop-client')
    HADOOP_JAVA_HOME = os.path.join(HADOOP_CLIENT_PATH, 'java6')
    SPARK_CLIENT_PATH = os.path.join(TOOL_PATH, 'spark-client')
    JAVA_HOME = os.path.join(TOOL_PATH, 'hadoop-client/java6')
    MYSQL_BIN = 'mysql'
    MOLA_CLIENT_PATH = os.path.join(TOOL_PATH, 'mola')

    #HDFS input and output path
    HDFS_ROOT_PATH = "/app/vs/ns-video/"
    #user behavior log path
    HDFS_MOBILE_PATH = os.path.join(HDFS_ROOT_PATH,
            'video-wise-data/vd-wise/android/behavior2/')

    HDFS_MOBILE_PLAY_LOG_PATH = os.path.join(HDFS_MOBILE_PATH, 'play/')
    HDFS_MOBILE_REC_LOG_PATH = os.path.join(HDFS_MOBILE_PATH, 'shortrec/')
    #derivant output path
    HDFS_DERIVANT_PATH = os.path.join(HDFS_ROOT_PATH, 'guming/{0}/'.format(PROJECT_NAME))



