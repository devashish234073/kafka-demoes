// receiver.js
const net = require('net');
const fs = require('fs');
const path = require('path');

const UPLOADS_DIR = path.join(__dirname, 'uploads');
if (!fs.existsSync(UPLOADS_DIR)) fs.mkdirSync(UPLOADS_DIR);

const server = net.createServer((socket) => {
  console.log('📤 Client connected');

  const filename = `upload_${Date.now()}.bin`;
  const writeStream = fs.createWriteStream(path.join(UPLOADS_DIR, filename));

  socket.pipe(writeStream);

  socket.on('end', () => {
    console.log(`✅ File saved: ${filename}`);
  });

  socket.on('error', (err) => {
    console.error('Socket error:', err);
  });
});

const PORT = 8000;
server.listen(PORT, () => {
  console.log(`📡 TCP server listening on port ${PORT}`);
});

// Simple HTTP endpoint to return a monotonic time value (seconds with subsecond precision)
// Useful for external timing clients to avoid wall-clock jumps.
const http = require('http');
const HTTP_PORT = 8001;

const httpServer = http.createServer((req, res) => {
  if (req.method === 'GET' && req.url === '/time') {
    // process.hrtime.bigint gives nanoseconds as BigInt
    const t = Number(process.hrtime.bigint()) / 1e9; // seconds with fractions
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ time: t }));
    return;
  }

  res.writeHead(404);
  res.end();
});

httpServer.listen(HTTP_PORT, () => {
  console.log(`📡 HTTP time endpoint listening on port ${HTTP_PORT} (/time)`);
});