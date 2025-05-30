接收机参数测试

#虚拟串口对：
socat -d -d pty,raw,echo=0 pty,raw,echo=0

重命名：
sudo ln -sf /dev/pts/3 /dev/ttyUSB0

监听：
cat /dev/ttyUSB0

发送：
echo "Hello from virtual USB" > /dev/pts/4


运行：
qemu-arm -L /usr/arm-linux-gnueabihf ./bin/paramtest /dev/ttyUSB0（串口名）