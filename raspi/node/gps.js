const request = require('request');
const options = {
  method: 'POST',
};

// Serial port to receive NEMA sentences from GPS module
var SerialPort = require('serialport');
var Readline = SerialPort.parsers.Readline;
//const GPS_PORT = '/dev/serial/by-id/usb-FTDI_FT230X_Basic_UART_DJ00LUHR-if00-port0';
const GPS_PORT = '/dev/serial/by-id/usb-FTDI_FT230X_Basic_UART_DJ00LTN7-if00-port0'
var port = new SerialPort(GPS_PORT, { baudRate: 9600 });
var parser = new Readline();

// Service ID
const SERVICE_ID = 'aicam'

// NEMA GPGGA sentence
const INVALID = '0'
const GPS_FIX = '1'
let fixQuality = INVALID
let numSatelites = 0;
let seaLevel = 0;
let geoid = 0;

// REST API URL
let url = 'http://127.0.0.1:18082/sensor/'+ SERVICE_ID + '/gps';

// dddmm.mmmm to Google map coordinates
function toGoogleMapCoords(coord) {
  let v = Number(coord)/100.0; 
  let ddd = Math.floor(v);
  let mmDotmmmm = (v - ddd) * 100.0;
  return ( ddd + ( mmDotmmmm / 60.0 ) ).toFixed(6);
}

port.pipe(parser);
parser.on('data', data => {
  if (/\$GPRMC/.test(data)) {
    console.log(data);
    let gprmc = data.split(',');
    let hms = gprmc[1].split('.')[0];
    let status = gprmc[2];
    let latitude = toGoogleMapCoords(gprmc[3]);

    let ns = gprmc[4];
    let longitude = toGoogleMapCoords(gprmc[5]);
    let ew = gprmc[6];
    let velocity = gprmc[7];
    let direction = gprmc[8];
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

    if (fixQuality == GPS_FIX) {
      options.url = url + '?longitude=' + ew + longitude + '&latitude=' + ns + latitude + '&numSatelites=' + numSatelites + '&sealevel=' + seaLevel + '&geoid=' + geoid + '&timeJst=' + hmsJST; 

      request(options, (err, res, body) => {
      //console.log(res);
      });
    }

  } else if (/\$GPGGA/.test(data)) {
    console.log(data);
    let gpgga = data.split(',');
    fixQuality = gpgga[6];
    numSatelites = gpgga[7];
    seaLevel = gpgga[9];
    geoid = gpgga[11];
    if (seaLevel == '') seaLevel = null;
    if (geoid == '') geoid = null;
    console.log('numSatelites: ', numSatelites);
    console.log('seaLevel: ', seaLevel);
    console.log('geoid: ', geoid);
  }
});
