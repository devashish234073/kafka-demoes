# kafka-demoes

Steps to setup and run sendfile-demo

Start the nodeapp that will recieve the file
```
cd sendfile-demo/node
node filereceiver.js
```

In another terminal sessions launch the sendfile demo and without sendfile one

```
cd sendfile-demo/cpp
./testfile-creator.sh
ls -lart testfile.bin
./compile-and-send-with-sendfile.sh
./send-without-sendfile.sh
```

<img width="1919" height="324" alt="image" src="https://github.com/user-attachments/assets/ac1c4924-f625-4593-a732-c0e641c62c17" />

More runs:

<img width="1908" height="756" alt="image" src="https://github.com/user-attachments/assets/16c9b851-07c1-4fcb-a014-899030704691" />

