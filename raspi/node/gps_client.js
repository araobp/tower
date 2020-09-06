const request = require('request');
const URL = 'http://127.0.0.1:18082/sensor/aicam';
const options = {
  method: 'POST',
};

var SerialPort = require('serialport');
var Readline = SerialPort.parsers.Readline;
//const DEVICE = '/dev/serial/by-id/usb-FTDI_FT230X_Basic_UART_DJ00LUHR-if00-port0';
const DEVICE = '/dev/serial/by-id/usb-FTDI_FT230X_Basic_UART_DJ00LTN7-if00-port0'

let seaLevel = 0;
let geoid = 0;

var port = new SerialPort(DEVICE, { baudRate: 9600 });
var parser = new Readline();

port.pipe(parser);
parser.on('data', data => {
  if (/\$GPRMC/.test(data)) {
    console.log(data);
    let gprmc = data.split(',');
    let hms = gprmc[1].split('.')[0];
    let status = gprmc[2];
    let latitude = gprmc[3];
    let ns = gprmc[4];
    let longitude = gprmc[5];
    let ew = gprmc[6];
    let velocity = gprmc[7];
    let direction = gprmc[8];
    if (longitude == '') {
      longitude = null;
    } else {
      longitude = (parseFloat(longitude)/100).toString();
    }
    if (latitude == '') {
      latitude = null;
    } else {
      latitude = (parseFloat(latitude)/100).toString();
    }
    if (ew == 'E' || ew == '') {
      ew = ''
    } else {
      ew = '-'
    };
    if (ns == 'N' || ns == '') {
      ns = ''
    } else {
      ns = '-'
    };

    let h = hms.substring(0,2);
    let m = hms.substring(2,4);
    let s = hms.substring(4,6);
    let hInt = parseInt(h) + 9;
    if (hInt > 24) hInt -= 24;
    if (hInt < 10) {
      h = '0' + hInt.toString();
    } else {
      h = hInt.toString();
    }
    let hmsJST = h + m + s;
    console.log('HHmmss: ', hmsJST);
    console.log('status: ', status);
    console.log('longitude', ns, longitude);
    console.log('latitude', ew, latitude);
    console.log('velocity: ', velocity);
    console.log('direction: ', direction);

    latitude = '139.748101';
    longitude = '35.649545';
    seaLevel = 5;
    geoid = 10;
    options.url = URL + '?longitude=' + ew + longitude + '&latitude=' + ns + latitude + '&sealevel=' + seaLevel + '&geoid=' + geoid + '&datetime=' + hmsJST; 

    request(options, (err, res, body) => {
      //console.log(res);
    });

  } else if (/\$GPGGA/.test(data)) {
    console.log(data);
    let gpgga = data.split(',');
    let numSatelites = gpgga[7];
    seaLevel = gpgga[9];
    geoid = gpgga[11];
    if (seaLevel == '') seaLevel = null;
    if (geoid == '') geoid = null;
    console.log('numSatelites: ', numSatelites);
    console.log('seaLevel: ', seaLevel);
    console.log('geoid: ', geoid);
  }
});
