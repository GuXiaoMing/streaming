"""
# @file mail.py
# @Synopsis  send email
# @author Ming Gu(guming02@baidu.com))
# @version 1.0
# @date 2015-09-08
"""

import commands
import socket
import getpass
import logging


class MailClient(object):
    """
    # @Synopsis  send mail
    """
    def __init__(self, log_name):
        self.logger = logging.getLogger(log_name)

    def send(self, receivers, title, content):
        """
        # @Synopsis  send mail
        #
        # @Args receivers list of receivers
        # @Args title
        # @Args content
        #
        # @Returns   succeeded or not
        """
        host_name = socket.gethostname()
        user_name = getpass.getuser()
        content = '{0}@{1}\t{2}'.format(user_name, host_name, content)

        bash_cmd = ' '.join(['echo "{0}"'.format(content),
            '| mail -s', '"{0}"'.format(title), ' '.join(receivers)])

        status, output = commands.getstatusoutput(bash_cmd)
        self.logger.debug('Returned {0}: {1}\n{2}'.format(status, bash_cmd, output))
        return status == 0

if __name__ == '__main__':
    import sys
    sys.path.append('..')
    from conf.init_logger import initLogger
    from conf.env_config import EnvConfig
    initLogger()

    mail_client = MailClient(EnvConfig.LOG_NAME)
    mail_client.send(['guming@itv.baidu.com'], 'test mail', 'test content')
