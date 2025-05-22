
/*
说明：该代码用于读取LS盒子数据，主要通过串口通信实现，主要提取“CC 55 00 00" 开头的后32位数据，用于转化为x,y,z,roll,pitch,yaw,sec,msec等数据。
    在建图时该数据为LS盒子的位姿数据。可以使用单雷达建图，也可以通过IMU辅助建图。

作者：石杨杨
时间：2025年3月1日
版本：1.0
功能：通过串口读取LS盒子数据


*/


#include <serial/serial.h> // 串口通信库
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <limits>
#include <unistd.h> // 
// 发送16进制数据
void sendHexData(serial::Serial& mySerial, const std::vector<uint8_t>& data) 
{
    mySerial.write(data);
    std::cout << "Sent Hex Data: ";
    for (uint8_t byte : data) 
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " "; // 打印发送的16进制数据
    }
    std::cout << std::dec << std::endl; // 恢复十进制输出
}


//无符号整数转为有符号整数
int UnsignToint(long long unsignedValue)
{
    int res;
    if(unsignedValue <= static_cast<unsigned int>(std::numeric_limits<int>::max()))
    {
        res = static_cast<int>(unsignedValue);
    }
    else
    {
        res = static_cast<int>(unsignedValue - 4294967296);
    }
    return res;
}

// 从十六进制字符串转换为无符号整数
void printDecimalData(const std::vector<uint8_t>& data) {

    std::vector<uint8_t> extractedData;
    // std::cout << "Converted Decimal Data: ";
    for (uint8_t byte : data) 
    {  
        extractedData.push_back(byte);
    }

    // 提取各个数据字段
    unsigned int xHex = extractedData[0] + extractedData[1]*256 +extractedData[2]*256*256+extractedData[3]*256*256*256 ;
    unsigned int yHex = extractedData[4] + extractedData[5]*256 +extractedData[6]*256*256+extractedData[7]*256*256*256 ;
    unsigned int zHex = extractedData[8] + extractedData[9]*256 +extractedData[10]*256*256+extractedData[11]*256*256*256 ;
    unsigned int rollHex = extractedData[12] + extractedData[13]*256 +extractedData[14]*256*256+extractedData[15]*256*256*256 ;
    unsigned int pitchHex = extractedData[16] + extractedData[17]*256 +extractedData[18]*256*256+extractedData[19]*256*256*256 ;
    unsigned int yawHex = extractedData[20] + extractedData[21]*256 +extractedData[22]*256*256+extractedData[23]*256*256*256 ;
    unsigned int secHex = extractedData[24] + extractedData[25]*256 +extractedData[26]*256*256+extractedData[27]*256*256*256 ;
    unsigned int msecHex = extractedData[28] + extractedData[29]*256 +extractedData[30]*256*256+extractedData[31]*256*256*256 ;

    // 转换为有符号整数
    double x = (double)UnsignToint(xHex)/1000;
    double y = (double)UnsignToint(yHex)/1000;
    double z = (double)UnsignToint(zHex)/1000;
    double roll = (double)UnsignToint(rollHex)/100;
    double pitch = (double)UnsignToint(pitchHex)/100;
    double yaw = (double)UnsignToint(yawHex)/100;
    int sec = UnsignToint(secHex);
    int msec = UnsignToint(msecHex);

    // 输出结果
    std::cout << "x: " << x << std::endl;
    std::cout << "y: " << y << std::endl;
    std::cout << "z: " << z << std::endl;
    std::cout << "roll: " << roll << std::endl;
    std::cout << "pitch: " << pitch << std::endl;
    std::cout << "yaw: " << yaw << std::endl;
    std::cout << "sec: " << sec << std::endl;
    std::cout << "msec: " << msec << std::endl;
    
    std::ofstream outFile("./data/pos.txt", std::ios::app); // Open in append mode
    if (outFile.is_open()) {
        // Write the data to the file
        outFile << sec << "."<<msec<<" "<< x <<" "<< y <<" "<< z <<" "<< roll <<" "<< pitch <<" "<< yaw <<std::endl;
        // outFile << "msec: " << msec << std::endl;
        // outFile << std::endl; // Add a blank line between records
        // Close the file
        outFile.close();
    } else {
        std::cerr << "Unable to open file" << std::endl;
    }

}

// 新增CRC校验函数
uint16_t crc16(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}
// 从数据中提取以 "CC 55 00 00" 开头的后32位数据
std::vector<uint8_t> extractData(const std::vector<uint8_t>& data) {
    
    std::vector<uint8_t> extractedData ;
    const std::vector<uint8_t> errorData = {0xFF};
    const std::vector<uint8_t> header = {0xCC, 0x55, 0x00, 0x00};
    const std::vector<uint8_t> footer = {0xE0, 0x0E}; // 新增结尾标识
    auto it = std::search(data.begin(), data.end(), header.begin(), header.end());

    if (it != data.end()) 
    {
        std::cout << "Found header: CC 55 00 00" << std::endl;

        // 计算数据头的结束位置
        size_t headerEnd = std::distance(data.begin(), it) + header.size();
      
        // 检查是否有足够的32个字节
        const size_t expectedLength = header.size() + 33 + footer.size();
        if (data.end() - it < expectedLength) {
            std::cerr << "Incomplete data package!" << std::endl;
            return errorData;
        }
        
        // 验证结尾标识
        auto footerStart = it + header.size() + 33;
        if (!std::equal(footer.begin(), footer.end(), footerStart)) {
            std::cerr << "Invalid footer!" << std::endl;
            return errorData;
        }
        
        // 提取数据部分（32字节）
        extractedData.assign(it + header.size(), it + header.size() + 33);

        // 校验CRC（最后2字节）
        // uint16_t receivedCrc = *(footerStart + footer.size()) | (*(footerStart + footer.size() + 1) << 8);
        // uint16_t calculatedCrc = crc16(&(*it), header.size() + 33 + footer.size());
        
        // if (receivedCrc != calculatedCrc) 
        // {
        //     // std::cerr << "CRC check failed! Received: " << receivedCrc 
        //     //          << " Calculated: " << calculatedCrc << std::endl;
        //     return errorData;
        // }
        return extractedData;
        
    } 
    

    return errorData;
}

// 读取16进制数据并处理
void readAndProcessHexData(serial::Serial& mySerial)
{
    static std::vector<uint8_t> persistent_buffer;  // 持久化缓冲区
    
    size_t bytesAvailable = mySerial.available();
    if (bytesAvailable > 0) 
    {
        // 读取新数据到临时缓冲区
        std::vector<uint8_t> temp_buffer(bytesAvailable);
        size_t bytesRead = mySerial.read(temp_buffer.data(), bytesAvailable);
        
        // 将新数据追加到持久化缓冲区
        persistent_buffer.insert(persistent_buffer.end(), 
                               temp_buffer.begin(), 
                               temp_buffer.begin() + bytesRead);
    }
    // 循环处理所有完整数据包
    // std::vector<uint8_t> buffer;
    // size_t bytesAvailable = mySerial.available();
    // if (bytesAvailable > 0) 
    // {
    //     buffer.resize(bytesAvailable);
    //     size_t bytesRead = mySerial.read(buffer.data(), bytesAvailable);

    //     std::cout << "Received Hex Data: ";
    //     for (size_t i = 0; i < bytesRead; ++i) 
    //     {
    //         std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[i]) << " ";
    //     }
    //     std::cout << std::dec << std::endl;

    // }

    // 提取并输出以 "CC 55 00 00" 开头的后32位数据
    std::vector<uint8_t> extractedData = extractData(persistent_buffer);

    if (extractedData.size() == 33) {

        printDecimalData(extractedData);
        // 清空处理过的数据
        persistent_buffer.erase(persistent_buffer.begin(), persistent_buffer.end() );
    }

}

int getUserInput() 
{
    int choice;
    std::cout << "Please select an option:" << std::endl;
    std::cout << "1. Start positioning" << std::endl;
    std::cout << "2. Stop positioning" << std::endl;
    std::cout << "3. Start mapping" << std::endl;
    std::cout << "4. Stop mapping" << std::endl;
    std::cout << "Please select an option:";
    std::cin >> choice;
    return choice;
}

int main() 
{
    // 打开串口
    // serial::Serial mySerial("/dev/ttyUSB0", 115200, serial::Timeout::simpleTimeout(1000)); // Linux
    serial::Serial* mySerial = new serial::Serial("/dev/ttyUSB0", 115200, serial::Timeout::simpleTimeout(1000));
    
    if (!mySerial->isOpen()) 
    {
        std::cerr << "Failed to open serial port!" << std::endl;
        return -1;
    }

    std::cout << "Serial port opened successfully!" << std::endl;

    // 发送命令，根据选择发送
    int choice = getUserInput();
    std::vector<uint8_t> hexData;
    switch (choice)
    {
        
        case 1:
            // Start positioning
            hexData = {
                0xA0, 0x0A, 0x12, 0x73, 0x4D, 0x4E, 0x20, 0x6D, 0x43, 0x68, 0x61, 0x6E, 0x67, 0x65, 0x53, 0x74,
                0x61, 0x74, 0x65, 0x20, 0x33, 0x5F, 0xE0, 0x0E
            };
            break;
        case 2:
            // Stop positioning
            hexData = {
                0xA0, 0x0A, 0x12, 0x73, 0x4D, 0x4E, 0x20, 0x6D, 0x43, 0x68, 0x61, 0x6E, 0x67, 0x65, 0x53, 0x74,
                0x61, 0x74, 0x65, 0x20, 0x34, 0x58, 0xE0, 0x0E
            };
            break;
        case 3:
            // Start mapping
            hexData = {
                0xA0, 0x0A, 0x12, 0x73, 0x4D, 0x4E, 0x20, 0x6D, 0x43, 0x68, 0x61, 0x6E, 0x67, 0x65, 0x53, 0x74,
                0x61, 0x74, 0x65, 0x20, 0x31, 0x5D, 0xE0, 0x0E
            };
            break;
        case 4:
            // Stop mapping
            hexData = {
                0xA0, 0x0A, 0x12, 0x73, 0x4D, 0x4E, 0x20, 0x6D, 0x43, 0x68, 0x61, 0x6E, 0x67, 0x65, 0x53, 0x74,
                0x61, 0x74, 0x65, 0x20, 0x32, 0x5E, 0xE0, 0x0E
            };
            break;
        default:
            std::cerr << "Invalid choice!" << std::endl;
            return -1;
    }
    // 发送16进制数据
    sendHexData(*mySerial, hexData);


    // 循环读取数据
    while (choice == 1 || choice == 3) 
    {
        // 读取16进制数据并处理
        readAndProcessHexData(*mySerial);

        usleep(100000); // 10ms延时
    }

    // 关闭串口
    // 修改4：显式释放串口资源
    mySerial->close();
    delete mySerial;
    return 0;
}