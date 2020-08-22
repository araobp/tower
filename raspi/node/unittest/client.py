import pprint
import requests
import base64
import time
import unittest

URL = 'http://localhost:18080{}'
header = {'Content-type': 'application/octet-stream'}

def _put(path, binary_data):
    return requests.put(URL.format(path), data=binary_data,
            headers=header, verify=False)

def _get(path):
    return requests.get(URL.format(path), verify=False)

def _pprint(resp, comment=None):
    print()
    if comment:
        print(comment)

    print('status code: {}'.format(resp.status_code))

    if resp.status_code == 200:
        try:
            pprint.pprint(resp.json())
        except:
            print(resp.text)
    else:
        print(resp.text)

with open('./gps.jpg', 'rb') as f:
    # read image data and convert it into Base64 string 
    # b64data = base64.b64encode(f.read()).decode('utf-8')
    binary_data = f.read()

timestamp = int(time.time())
device0 = 'device0'
device1 = 'device{}'.format(str(timestamp))
device2 = 'device{}'.format(str(timestamp+10))

class TestSequence(unittest.TestCase):

    def setUp(self):
        r = _put('/devices/{}/{}'.format(device0, '20191026134508-gps.jpg'), binary_data)
        self.assertEqual(r.status_code, 200)

        r = _put('/devices/{}/{}'.format(device0, '20191025134504-gps.jpg'), binary_data)
        self.assertEqual(r.status_code, 200)

        r = _put('/devices/{}/{}'.format(device0, '20191025134508-gps.jpg'), binary_data)
        self.assertEqual(r.status_code, 200)

    def test_get_devices(self):
        r = _get('/devices')
        _pprint(r)
        self.assertEqual(r.status_code, 200)

    def test_get_all(self):
        r = _get('/devices/{}'.format(device0))
        _pprint(r)
        self.assertEqual(r.status_code, 200)

    def test_get_latest(self):
        r = _get('/devices/{}?option=latest'.format(device0))
        _pprint(r)
        self.assertEqual(r.status_code, 200)

    def test_put_file_new_device1(self):
        r = _put('/devices/{}/{}'.format(device1, 'gps.jpg'), binary_data)
        self.assertEqual(r.status_code, 200)

    def test_put_file_new_device2(self):
        r = _put('/devices/{}/{}'.format(device2, 'gps.jpg'), binary_data)
        self.assertEqual(r.status_code, 200)

if __name__ == '__main__':
    unittest.main(verbosity=2, warnings='ignore')

