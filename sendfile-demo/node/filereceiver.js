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