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

Tests with 50 MB files:

<img width="1908" height="917" alt="image" src="https://github.com/user-attachments/assets/64b9c9db-1e69-4355-8f2f-a03ac2ab6b94" />

Tests with 250 MB files:

<img width="1919" height="420" alt="image" src="https://github.com/user-attachments/assets/07e86210-b139-45af-b148-7bb2aabc2a77" />


To create 250 MB file change the value and run:
<img width="1511" height="107" alt="image" src="https://github.com/user-attachments/assets/c4205fc5-b524-4564-b53e-9adf889964b2" />

<img width="894" height="207" alt="image" src="https://github.com/user-attachments/assets/5ed15fe9-9b8f-4cdf-834e-2d9b2bcd3b23" />

With the minimal nodejs file:

<img width="1197" height="191" alt="image" src="https://github.com/user-attachments/assets/c1e06829-bd3a-4dfd-841b-153e0d421f6a" />

