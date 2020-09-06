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

app.get('/broadcast/:serivceId', (req, res) => {
	let serivceId = req.params.serivceId;

  res.on('close', () => {
    if (serivceId in watchers) {
      console.log("A watcher of " + serivceId.toString() + " is closed");
      watchers[serivceId] = watchers[serivceId].filter ( v => res !== v );
      if (watchers[serivceId].length == 0) {
        delete watchers.serivceId;
        console.log(serivceId.toString() + ' has no watchers');
      }
    }
  });

  if (serivceId in watchers) {
    watchers[serivceId].push(res);
  } else {
    watchers[serivceId] = [res]
  }


  console.log(serivceId.toString() + ' connected');

  res.writeHead(200, {
      'Cache-Control': 'no-cache, no-store, max-age=0, must-revalidate',
      'Connection': 'keep-alive',
      // here we define the boundary marker for browser to identify subsequent frames
      'Content-Type': `multipart/x-mixed-replace;boundary="${BOUNDARY}"`,
      'Expires': 'Thu, Jan 01 1970 00:00:00 GMT',
      'Pragma': 'no-cache'
  });
});

app.post('/broadcast/:serivceId', (req, res) => {
  let serivceId = req.params.serivceId;
  sendResp(res, false, null);

  // Broadcast the JPEG image to all the watchers
  let buf = req.body;
	if (serivceId in watchers) {
    let l = watchers[serivceId];
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

app.post('/sensor/:serivceId/gps', (req, res) => {
  let serivceId = req.params.serivceId;
  let latitude = req.query.latitude;
  let longitude = req.query.longitude;
  let numSatelites = req.query.numSatelites;
  let seaLevel = req.query.sealevel;
  let geoid = req.query.geoid;
  let hmsJST = req.query.datetime;
  console.log(req.query);
  sendResp(res, false, null);
});

app.get('/sensor/:serviceId/gps', (req, res) => {
  // TODO
});

// Start this broadcaster
const server = app.listen(PORT, () => {
  console.log('Motion JPEG Broadcaster has started at port: ' + PORT);
});

