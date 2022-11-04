'''
This file is intended to test the connections of the API.
You will nedd an external api_config.ini file with the IP
of the 
'''

import configparser
config = configparser.ConfigParser()
config.read('api_config.ini')

BASE_URL = "http://{}/api/v1/output/".format(config['API']['IP'])

print(BASE_URL)