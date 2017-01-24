import os
import time

import requests

from ab_log import *

init_ab_logs(pongo.Logs())


@log_if_errors(name='test', flush_level=logging.INFO)
def test_log(log):
    log.warning('aaaa')


if __name__ == '__main__':
    # test log
    test_log()
    # test file
    f = open('work_file', 'ar+')
    f.write('%d\n' % os.getpid())
    # show pid
    print os.getpid()
    # test class
    print pongo.Config().get_config()
    print pongo.Cookies().load_cookie("")
    # test sleep
    time.sleep(2)
    # test request
    print requests.get("http://www.baidu.com").text
