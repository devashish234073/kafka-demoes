// minimalfilereceiver.js
// Minimal TCP sink that discards incoming file data and provides a /time HTTP endpoint.
// No file writes or console logging for the TCP path — useful for benchmarking sender throughput.

const net = require('net');
const http = require('http');
const fs = require('fs');
const path = require('path');

const TCP_PORT = 8000; // same port your senders use
const HTTP_PORT = 8001; // keep the same /time endpoint port

const server = net.createServer((socket) => {
  // Do not log. Consume and discard data to avoid disk I/O.
  socket.on('data', () => {
    // no-op: implicitly discard
  });

  // Ensure we swallow errors silently to keep the sink running
  socket.on('error', () => {});
});

server.listen(TCP_PORT, () => {
  // Minimal startup message
  console.log(`📡 Minimal TCP sink listening on port ${TCP_PORT}`);
});

// ensure uploads dir exists so cleanup endpoint can operate on the same folder
const UPLOADS_DIR = path.join(__dirname, 'uploads');
if (!fs.existsSync(UPLOADS_DIR)) fs.mkdirSync(UPLOADS_DIR);

// Keep the monotonic time endpoint for timing clients
const httpServer = http.createServer((req, res) => {
  // /time endpoint
  if (req.method === 'GET' && req.url === '/time') {
    const t = Number(process.hrtime.bigint()) / 1e9; // seconds with fractions
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ time: t }));
    return;
  }

  // /cleanup endpoint - delete files under uploads
  if (req.method === 'GET' && req.url === '/cleanup') {
    fs.readdir(UPLOADS_DIR, (err, files) => {
      if (err) {
        res.writeHead(500, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ error: String(err) }));
        return;
      }
      if (!files.length) {
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ deleted: 0 }));
        return;
      }
      let remaining = files.length;
      let deleted = 0;
      const errors = [];
      files.forEach((f) => {
        const p = path.join(UPLOADS_DIR, f);
        fs.unlink(p, (e) => {
          if (e) errors.push({ file: f, error: String(e) });
          else deleted++;
          remaining--;
          if (remaining === 0) {
            res.writeHead(200, { 'Content-Type': 'application/json' });
            res.end(JSON.stringify({ deleted, errors }));
          }
        });
      });
    });
    return;
  }

  res.writeHead(404);
  res.end();
});

httpServer.listen(HTTP_PORT, () => {
  console.log(`📡 HTTP time endpoint listening on port ${HTTP_PORT} (/time)`);
});
