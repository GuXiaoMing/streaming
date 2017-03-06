"""
# @file spark_submit.py
# @Synopsis  submit task to spark cluster
# @author Ming Gu(guming02@baidu.com))
# @version 1.0
# @date 2015-09-17
"""

import os
import commands
import logging

class SparkSubmitter(object):
    """
    # @Synopsis  pack spark-submit into a python module
    """

    def __init__(self, client_path, log_name):
        self.client_path = client_path
        self.bin_path = os.path.join(client_path, 'bin/spark-submit')
        self.logger = logging.getLogger(log_name)

    def submit(self, main_program, py_files, master='yarn'):
        """
        # @Synopsis  spark-submit
        #
        # @Args main_program_path either absolute path or relative path to the
        # current working path
        # @Args run_locally --master local option of spark-submit script
        #
        # @Returns  succeeded or not
        """
        parameter_list = [self.bin_path]
        parameter_list.append('--master {0}'.format(master))
        parameter_list.append('--py-files ' + py_files)
        parameter_list.append(main_program)
        bash_cmd = ' '.join(parameter_list)
        status, output = commands.getstatusoutput(bash_cmd)
        self.logger.debug('Returned {0}: {1}\n{2}'.format(status, bash_cmd, output))

    def submit_with_modules(self, main_program, modules_father_path,
            modules=None, run_locally=False):
        """
        # @Synopsis  spark-submit with modules uploaded after zipped
        # @Args main_program
        # @Args modules_father_path
        # @Args modules
        # @Args run_locally
        # @Returns   succeeded or not
        """
        if modules is None:
            modules = ['conf', 'bll', 'dao']
        pwd = os.getcwd()
        os.chdir(modules_father_path)
        for module_name in modules:
            bash_cmd = 'zip -rq {0}.zip {0}/*'.format(module_name)
            status, output = commands.getstatusoutput(bash_cmd)
        os.chdir(pwd)

        zip_names = map(lambda x: '{0}/{1}.zip'.format(modules_father_path, x), modules)
        master = 'local[4]' if run_locally else 'yarn'
        return self.submit(main_program, ','.join(zip_names), master)

if __name__ == '__main__':
    pass
