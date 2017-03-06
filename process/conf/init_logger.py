"""
# @file init_logger.py
# @Synopsis  init logger
# @author Ming Gu(guming02@baidu.com))
# @version 1.0
# @date 2015-09-19
"""
import sys
import logging
import logging.handlers
from conf.env_config import EnvConfig
from dao.mail import MailClient

class MailHandler(logging.Handler):
    """
    # @Synopsis  customized handler, to email critical log
    """

    def emit(self, record):
        """
        # @Synopsis  override logging.Handler emit method, the action when receive
        # the logging record
        #
        # @Args record
        #
        # @Returns nothing
        """
        msg = self.format(record)
        mail_client = MailClient(EnvConfig.LOG_NAME)
        mail_client.send(EnvConfig.MAIL_RECEIVERS, 'PROGRAM ALARM', msg)


def initLogger():
    """
    # @Synopsis  init logger
    # @Returns   None
    """
    logger = logging.getLogger(EnvConfig.LOG_NAME)
    logger.setLevel(logging.DEBUG)
    file_hdlr = logging.handlers.TimedRotatingFileHandler(
            EnvConfig.GENERAL_LOG_FILE, when='D', backupCount=7)
    stdout_hdler = logging.StreamHandler(sys.stdout)
    email_hdler =  MailHandler()
    email_hdler.setLevel(logging.ERROR)
    formatter = logging.Formatter('%(asctime)s %(levelname)s: %(message)s',
            "%Y-%m-%d %H:%M:%S")
    file_hdlr.setFormatter(formatter)
    stdout_hdler.setFormatter(formatter)
    email_hdler.setFormatter(formatter)
    logger.addHandler(file_hdlr)
    logger.addHandler(stdout_hdler)
    logger.addHandler(email_hdler)

if __name__ == '__main__':
    InitLogger()
