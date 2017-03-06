"""
# @file redis.py
# @Synopsis  redis client
# @author Ming Gu(guming@itv.baidu.com))
# @version 1.0
# @date 2016-10-21
"""
import os
import commands
import logging

class RedisClient(object):
    """
    # @Synopsis  call external redis client and write data
    """
    def __init__(self, client_path, log_name):
        self.client_path = client_path
        self.bin_path = os.path.join(client_path, 'bin')
        self.logger = logging.getLogger(log_name)

    def set_ex(key, value, expire_secs):
        """
        # @Synopsis  redis set_ex method
        # @Args key
        # @Args value
        # @Args expire_secs
        # @Returns  succeeded or not
        """
        command_path = os.path.join(client_path, 'set_ex')
        cwd = os.getcwd()
        set_cmd = ' '.join([command_path, key, value, str(expire_secs)])
        full_cmd = '&&'.join(['cd {0}'.format(self.bin_path),
            set_cmd, 'cd {0}'.format(cwd)])
        status, output = commands.getstatusoutput(full_cmd)
        self.logger.debug('Returned {0}: {1}\n{2}'.format(status, full_cmd, output))
        if status != 0:
            return False
        return True
