const express = require('express');
const fs = require('fs');

const app = express();
app.use(express.urlencoded({ extended: true }));
app.use(express.raw({ inflate: true, limit: '100mb', type: 'image/jpeg' }));

const PORT = 18082;

function sendResp(res, err, doc) {
  if (err) {
    res.status(doc.status).send(doc.reason);
  } else {
    if (doc) {
      res.send(doc);
    } else {
      res.send();
    }
  }
}

let watchers = {};
const BOUNDARY = "broadcaster";

let devices = {};

app.get('/broadcast/:deviceId', (req, res) => {
	let deviceId = req.params.deviceId;

  res.on('close', () => {
    if (deviceId in watchers) {
      console.log("A watcher of " + deviceId.toString() + " is closed");
      watchers[deviceId] = watchers[deviceId].filter ( v => res !== v );
      if (watchers[deviceId].length == 0) {
        delete watchers.deviceId;
        console.log(deviceId.toString() + ' has no watchers');
      }
    }
  });

  if (deviceId in watchers) {
    watchers[deviceId].push(res);
  } else {
    watchers[deviceId] = [res]
  }


  console.log(deviceId.toString() + ' connected');

  res.writeHead(200, {
      'Cache-Control': 'no-cache, no-store, max-age=0, must-revalidate',
      'Connection': 'keep-alive',
      // here we define the boundary marker for browser to identify subsequent frames
      'Content-Type': `multipart/x-mixed-replace;boundary="${BOUNDARY}"`,
      'Expires': 'Thu, Jan 01 1970 00:00:00 GMT',
      'Pragma': 'no-cache'
  });
});

app.post('/broadcast/:deviceId', (req, res) => {
  let deviceId = req.params.deviceId;
  sendResp(res, false, null);

  // Broadcast the JPEG image to all the watchers
  let buf = req.body;
	if (deviceId in watchers) {
    let l = watchers[deviceId];
    l.forEach( r => {
      r.write(`--${BOUNDARY}\r\n`);
      r.write('Content-Type: image/jpeg\r\n');
      r.write(`Content-Length: ${buf.length}\r\n`);
      r.write('\r\n');
      r.write(buf, 'binary');
      r.write('\r\n');
    });
  }
});

app.post('/sensor/:deviceId', (req, res) => {
  let deviceId = req.params.deviceId;
  let latitude = req.query.latitude;
  let longitude = req.query.longitude;
  let seaLevel = req.query.sealevel;
  let geoid = req.query.geoid;
  let hmsJST = req.query.datetime;
  devices[deviceId] = req.query; 
  console.log(devices);
  sendResp(res, false, null);
});

// Start this broadcaster
const server = app.listen(PORT, () => {
  console.log('Motion JPEG Broadcaster has started at port: ' + PORT);
});

