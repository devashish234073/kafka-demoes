# kafka-demoes

Steps to setup and run sendfile-demo

Start the nodeapp that will recieve the file
```
cd sendfile-demo/node
node filereceiver.js
```

In another terminal session launch the sendfile demo

```
cd sendfile-demo/cpp
./testfile-creator.sh
ls -lart testfile.bin
./compile-and-run.sh
```