// ls_usb.h
#ifndef LS_USB_H
#define LS_USB_H

#include <serial/serial.h>
#include <vector>

// 发送16进制数据
void sendHexData(serial::Serial& mySerial, const std::vector<uint8_t>& data);

// 无符号整数转为有符号整数
int UnsignToint(long long unsignedValue);

// 从数据中提取以 "CC 55 00 00" 开头的后32位数据
std::vector<uint8_t> extractData(const std::vector<uint8_t>& data);

// 读取16进制数据并处理
void readAndProcessHexData(serial::Serial& mySerial);

// 打印十进制数据
void printDecimalData(const std::vector<uint8_t>& data);

#endif // LS_USB_H