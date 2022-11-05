'''
This program is intended to test 🐦SmartOpenPark API connections.

You will need an external api_config.ini file with the API IP and key.
This file is normaly provided by me: 
📧 @alvarogd16 - alvarogarcia16102000@gmail.com

If you need to modify or create a new one the format is like
[API]
IP  = XX.XX.XX.XX
key = NNNNNNN

⚠️ Its important to execute this file in the same folder with the config file

TODO Add a licence
'''

import configparser
import time
import json
import sys
import requests

PARAMS_ERR = '''Error in the format of the program.
Usage: python3 testAPI.py MODE [MODE_OPTIONS]
Try 'python3 testAPI.py help\''''

CONFIG_ERR = '''Error in the config file.
API sections doesnt exists.
Try add a API section or check the config info in the file's beginning.'''

CONFIG_ERR_API = '''Error in the config file.
API sections is incorrect. IP or KEY is missing.
Try add a IP or KEY, or check the config info in the file's beginning.'''

GET_ERR = '''Probably the name of the sensos doesnt exists or the server is not working.
Try obtain the sensors with get_sensors option or try later this request.'''

HELP_MSG = '''testAPI program
This program is intended to test SmartOpenPark proyect API.

Usage: python3 testAPI.py MODE [MODE_OPTIONS]

Modes:
    help: get help about the program.
    get_sensor: get sensor data in a range of dates.
                The dates are hardcoded in the getSensorData function.
        Options: SENSOR_NAME
        Usage: python3 testAPI.py get_sensor [SENSOR_NAME]'''

# TODO Add get_sensors, create and delete features
MODES = ['help', 'get_sensor', 'get_sensors', 'create', 'delete']

BASE_URL = "http://{}/api/v1/output/"
GET_JSON='{{"info": {{"api_key": "{}","device": "{}","from":"{}", "to":"{}"}} }}'
METHODS = {'get': 'json_read'}
DEFAULT_SENSOR = 'SMARTOPENPARK_PRUEBAS_1'

'''
GMT Time format (YYYY-MM-DDTHH-MM-SS)
    Y -> Year, M -> Month, D -> Day
    H -> Hour, M -> Minutes, S -> Seconds 
'''
def getCurrentTime() -> str:
    t = time.localtime()
    return '{}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}'.format(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)

def showHelp():
    print(HELP_MSG)

'''
Get and print sensor data in a JSON format
Each entry is like:
{
    "created_at": STRING_DATE,
    "data": {
        "humidity": number,
        "temperature": number,
        ...
    }
}
'''
def getSensorData(base_url: str, api_key: str, sensor_name: str):
    date_from = "2020-06-07T00:00:00"
    date_to   = "2023-06-09T00:00:00"
    json_query = GET_JSON.format(api_key, sensor_name, date_from, date_to)
    
    url = base_url + METHODS['get'] + '?json=' + json_query
    print('Getting sensor data with this url:\n' + url)

    res = requests.get(url)
    print('Status code', res.status_code)
    if res.status_code == 200:
        print('Json response: ', json.dumps(json.loads(res.content), indent=4))
    else:
        print(GET_ERR)

def main():
    if len(sys.argv) < 2 or sys.argv[1] not in MODES:
        print(PARAMS_ERR)
        return

    '''You can get more info about config file in the header of this file'''
    config = configparser.ConfigParser()
    config.read('api_config.ini')
    if 'API' not in config.sections():
        print(CONFIG_ERR)
        return
    if not config.has_option('API', 'IP') or not config.has_option('API', 'KEY'):
        print(CONFIG_ERR_API)
        return

    '''The IP of the API is in the config file for security. Same with the API key.'''
    URL_WITH_IP = BASE_URL.format(config['API']['IP'])
    API_KEY = config['API']['KEY']

    '''
    The diferents modes of the program.
    If you add a new one dont fotget to include in MODES array.
    '''
    mode = sys.argv[1]
    if mode == 'help':
        showHelp()
    elif mode == 'get_sensor':
        sensor_name = sys.argv[2] if len(sys.argv) > 2 else DEFAULT_SENSOR
        getSensorData(URL_WITH_IP, API_KEY, sensor_name)

main()