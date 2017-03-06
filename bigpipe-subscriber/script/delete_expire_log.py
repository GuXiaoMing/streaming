"""
# @file delete_expire_log.py
# @Synopsis  delete expire log, to save storage resource
# @author Ming Gu(guming@itv.baidu.com))
# @version 1.0
# @date 2016-10-09
"""

import os
import re
import time
from datetime import datetime

expire_days = 3
files = os.listdir('.')
pattern = 'bigpipe\.log\.(\d+)'

for file_name in files:
    m = re.match(pattern, file_name)
    if m is not None:
        date = m.group(1)[:8]
        t = time.strptime(date, '%Y%m%d')
        dt = datetime.fromtimestamp(time.mktime(t))
        today = datetime.today()
        time_delta = today - dt
        if time_delta.days >= expire_days:
            os.remove(file_name)
