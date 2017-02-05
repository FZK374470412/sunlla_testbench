#! /usr/bin/python
################################################################################
"""sunlla_i2c_test.py will set simple operations possibly to show you a example,
   you can add new python test script for your module like this script.
"""
__author__ = "Vedic.Fu"
__copyright__ = "Free"
__credits__ = ["None"]
__license__ = "GPL"
__version__ = "1.0"
__maintainer__ = "Vedic.Fu"
__email__ = "FZKmxcz@163.com"
__status__ = ""
################################################################################

import time, sessionlib

errs = []
result = {}
obj = sessionlib.create_session()

# test 1: use exe_method
method = {'cmd': 'I2C_OPEN', 'I2C_dev': 0}
err = sessionlib.exe_method(obj, method) 
if err:
	print 'I2C_OPEN fail!'
	errs.append(1)

# test 2: use exe_method
err = sessionlib.exe_method(obj, {'cmd': 'I2C_START', 'I2C_dev': 1}) 
if err:
	print 'I2C_START fail!'
	errs.append(1)

# test 3: use exe_method_resp to get back parameter
err, result = sessionlib.exe_method_resp(obj, {'cmd': 'I2C_CLOSE', 'I2C_dev': 2}) 
if err:
	print 'I2C_CLOSE fail!'
	errs.append(1)
else:
	print result

# test 4: use exe_method_list test more than one method
method_list = [
	{'cmd': 'I2C_OPEN', 'I2C_dev': 0},
	{'cmd': 'I2C_START', 'I2C_dev': 1},
]
err = sessionlib.exe_method_list(obj, method_list) 
if err:
	print 'method_list fail!'
	errs.append(1)

sessionlib.exe_method_done(obj, errs)
