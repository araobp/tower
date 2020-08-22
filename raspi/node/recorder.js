const cluster = require('express-cluster');

const NUM_PROCESSES = 1;

cluster(function(worker) {
  const express = require('express');
  const multer = require('multer');
	const os = require('os');
  const fs = require('fs');
  const path = require('path');

  const SSDPATH = '/media/pi/SSD-PLU3'
  const PATH_DEVICES = path.join(SSDPATH, '/devices')

  const app = express();
  app.use(express.urlencoded({ extended: true }));

  const PORT = 18080;

	const ifaces = os.networkInterfaces();

	function localAddress() {
		let address = null
		Object.keys(ifaces).forEach(ifname => {
			ifaces[ifname].forEach(iface => {
				if (iface.family == 'IPv4' && iface.internal == false) {
					address = iface.address;
				}
			})
		})
		return address;
	}

	console.log(localAddress());
	console.log(PATH_DEVICES);

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

  app.get('/devices', (req, res) => {
      console.log('[get@' + worker.id.toString() +'] ' + PATH_DEVICES)
      fs.readdir(PATH_DEVICES, (err, dirs) => {
          if (err) {
              let doc = { status: 404, reason: 'Not Found' }
              sendResp(res, true, doc);
          } else {
              let doc = dirs;
              sendResp(res, false, doc);
          }
      });
  });

  app.get('/devices/:device', (req, res) => {
    let device = req.params.device
    let option = req.query.option
    let pathDevicesDevice = path.join(PATH_DEVICES, device);
    console.log('[get] ' + pathDevicesDevice);
    
    fs.readdir(pathDevicesDevice, (err, files) => {
      let latest = '0';
      if (err) {
        console.log('New device: ' + device);
        sendResp(res, false, latest);
      } else {
        let doc = files.sort();
        if (doc == undefined || doc.length == 0) {
          latest = '0';
        } else {
          latest = doc[doc.length - 1];
        }
        if (doc.length > 1) {
          if (latest.split('.')[2] == 'uploading') {
            // Remove incomplete upload
            fs.unlink(path.join(pathDevicesDevice, latest), err => {
                if (err) console.log(err);
            });
            latest = doc[doc.length - 2];
          }
        }
        if (option == 'last_timestamp') {
          latest = latest.split('-')[0];
        }
        console.log('...' + latest);
        sendResp(res, false, latest);
      }
    });
  });

  let storage = multer.diskStorage({
    destination: function (req, file, cb) {
      let device = req.params.device
      let pathDevice = path.join(PATH_DEVICES, device);
      if (!fs.existsSync(pathDevice)) {
        fs.mkdirSync(pathDevice)
      }
      cb(null, pathDevice);
      console.log(req.headers);
    },
    filename: function(req, file, cb) {
      let device = req.params.device
      let timestamp = Math.floor(new Date() / 1000).toString()
      req.timestamp = timestamp
      let filename = file.originalname + '.uploading.' + timestamp;
      cb(null, filename);
      let pathDevice = path.join(PATH_DEVICES, device);
      let pathDeviceFilename = path.join(pathDevice, filename)
      console.log('[post@' + worker.id.toString() + '] ' + pathDevice)
      console.log(filename)
    }
  });

  let upload = multer({ 
    storage: storage,
    onError: function(err, next) {
      console.log('upload error: ', err);
      next(err);
    }
  });

  app.post('/devices/:device', upload.single('file'), (req, res, next) => {
      let device = req.params.device
      let pathDevice = path.join(PATH_DEVICES, device);
      let filename = req.file.originalname;
      let pathDeviceFilename = path.join(pathDevice, filename);
      let pathDeviceFilenameUploading = path.join(pathDevice, filename + '.uploading.' + req.timestamp);
      fs.rename(pathDeviceFilenameUploading, pathDeviceFilename, err => {
          if (err) console.log(err);
      });
      sendResp(res, false, null);
  });

  const server = app.listen(PORT, () => {
      console.log('File server started (worker.id: ' + worker.id + ')');
  });

}, {count: NUM_PROCESSES});

