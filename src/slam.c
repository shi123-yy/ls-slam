#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <proj.h>
#include <math.h>
// #include <math_operation.h>
#define DEG2RAD      0.017453292519943
#define RAD2DEG      57.29577951308232
// #define LOV_PI       3.141592653589793

typedef struct
{
	double x;
	double y;
}VECTOR;

// 打开串口
int open_serial_port(const char *port_name, int baud_rate) 
{
    int fd = open(port_name, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("Failed to open serial port");
        return -1;
    }

    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, baud_rate);
    cfsetospeed(&options, baud_rate);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    tcsetattr(fd, TCSANOW, &options);
    return fd;
}

// 发送十六进制数据
void send_hex_data(int fd, const uint8_t *data, size_t length) 
{
    write(fd, data, length);
    printf("Sent Hex Data: ");
    for (size_t i = 0; i < length; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");
}

// 无符号整数转为有符号整数
int unsign_to_int(uint32_t unsigned_value) 
{
    if (unsigned_value <= (uint32_t)INT32_MAX) 
    {
        return (int)unsigned_value;
    } else {
        return (int)(unsigned_value - 4294967296);
    }
}

//UMT转GPS
void utm2gps(double easting, double northing, int zone, int is_north, double *latitude, double *longitude)
{
    // 创建PROJ上下文
    PJ_CONTEXT *ctx = proj_context_create();

    // 构建UTM投影字符串
    char utm_proj_str[100];
    snprintf(utm_proj_str, sizeof(utm_proj_str), 
            "+proj=utm +zone=%d +%s +ellps=WGS84",
            zone, is_north ? "north" : "south");

    // 定义坐标系转换
    PJ *utm_to_wgs84 = proj_create_crs_to_crs(
        ctx,
        utm_proj_str,                // 源坐标系：UTM
        "+proj=longlat +datum=WGS84", // 目标坐标系：WGS84经纬度
        NULL                         // 可选区域限制
    );

    if (!utm_to_wgs84) {
        fprintf(stderr, "转换器创建失败: %s\n", proj_errno_string(proj_context_errno(ctx)));
        proj_context_destroy(ctx);
        return 1;
    }

    // 创建坐标点结构体（输入单位为米）
    PJ_COORD input = proj_coord(easting, northing, 0, 0);

    // 执行坐标转换
    PJ_COORD output = proj_trans(utm_to_wgs84, PJ_FWD, input);

    // 输出结果（经度、纬度）
    printf("WGS-84 经度: %.9f°\n", output.lp.lam);
    printf("WGS-84 纬度:  %.9f°\n", output.lp.phi);

    // 清理资源
    proj_destroy(utm_to_wgs84);
    proj_context_destroy(ctx);
}
// 从十六进制字符串转换为无符号整数
void print_decimal_data(const uint8_t *data) 
{
    uint32_t xHex = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    uint32_t yHex = data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24);
    uint32_t zHex = data[8] | (data[9] << 8) | (data[10] << 16) | (data[11] << 24);
    uint32_t rollHex = data[12] | (data[13] << 8) | (data[14] << 16) | (data[15] << 24);
    uint32_t pitchHex = data[16] | (data[17] << 8) | (data[18] << 16) | (data[19] << 24);
    uint32_t yawHex = data[20] | (data[21] << 8) | (data[22] << 16) | (data[23] << 24);
    uint32_t secHex = data[24] | (data[25] << 8) | (data[26] << 16) | (data[27] << 24);
    uint32_t msecHex = data[28] | (data[29] << 8) | (data[30] << 16) | (data[31] << 24);

    double x = (double)unsign_to_int(xHex) / 1000;
    double y = (double)unsign_to_int(yHex) / 1000;
    double z = (double)unsign_to_int(zHex) / 1000;
    double roll = (double)unsign_to_int(rollHex) / 100;
    double pitch = (double)unsign_to_int(pitchHex) / 100;
    double yaw = (double)unsign_to_int(yawHex) / 100;
    int sec = unsign_to_int(secHex);
    int msec = unsign_to_int(msecHex);

    printf("x: %f\n", x);
    printf("y: %f\n", y);
    printf("z: %f\n", z);
    printf("roll: %f\n", roll);
    printf("pitch: %f\n", pitch);
    printf("yaw: %f\n", yaw);
    printf("sec: %d.%d\n", sec,msec);
    // printf("msec: %d\n", msec);

    FILE *outFile = fopen("./data/pos.txt", "a");
    if (outFile != NULL) {
        fprintf(outFile, "%d.%d %f %f %f %f %f %f\n", sec, msec, x, y, z, roll, pitch, yaw);
        fclose(outFile);
    } else {
        fprintf(stderr, "Unable to open file\n");
    }
    double latitude=0;
    double longitude = 0;
    double real_x = x + 336.54;
    double real_y = y + 388.18;
    // 定义UTM坐标参数
    int zone = 50;                // UTM区域号
    int is_north = 1;
    // 调用UTM转GPS函数
    utm2gps(real_x, real_y, zone, is_north, &latitude, &longitude);
}

//
/*************************************************************************
*函数说明：高斯坐标xy转经纬度
*参数：
*           guassx           -I    高斯x
*           guassy           -I    高斯y
*           loa              -O    经纬度
*
*返回：
*      无
*
*备注：分度带随机中央经线，代号按6°带计算，loa->x：经度；loa->y：纬度
*************************************************************************/
void gaussInverseFun(double guassx, double guassy, double cen_lon, VECTOR *loa)
{
	//测试：六度带，lon：119.15850723,lat：36.56533501,Y：20693233.831,X：4050443.643
	int  n  = 0;
	double L0 = 0;
	double x  = 0;
	double y  = 0;
	double e2 = 0;
	double e1 = 0;
	double B1 = 0;
	double F  = 0;
	double B2 = 0;
	double B3 = 0;

	double Bf  = 0;
	double W   = 0;
	double M   = 0;
	double N   = 0;
	double A   = 0;
	double H   = 0;
	double I   = 0;
	double t   = 0;
	double B   = 0;
	double C   = 0;
	double nn  = 0;

	double lat = 0;
	double lon = 0;
	
	
	n  = (int)(guassy / 1000000.0); 
    L0 = cen_lon; 

	x = guassx;
	y = guassy - n * 1000000.0 - 500000.0;


	e2 = 0.00669437999014;
	e1 = sqrt(pow((6378137.0 / 6356752.3142),2) - 1);
	B1 = x / 6367449.14574372;

	while (1)
	{
	    F = -32077.0172984966*0.5*sin(2.0*B1)+67.3303988883*0.25*sin(4.0*B1)-0.13188597827/6.0*sin(6.0*B1)+0.000244629482*0.125*sin(8.0*B1);
	    B2 = (x - F) / 6367449.14574372;
	    B3 = B1;
	    B1 = B2;
	    
    	if (fabs(B3-B2) <= 2.78e-9)
    	{
			break;
    	}
	}
	Bf = B2;
	W  = sqrt(1-e2*pow(sin(Bf),2));
	M  = 6378137.0 * (1-e2) / pow(W,3);
	N  = 6378137.0 / W;
	A  = y / N;
	H  = pow(A,3);
	I  = pow(A,5);
	t  = tan(Bf);
	nn = e1 * cos(Bf);
	B  = t * t;
	C  = nn * nn;
	
	lat = Bf-y*t*A/(2.0*M)+y*t*H/(24.0*M)*(5.0+3.0*B+C-9.0*B*C)-y*t*I/(720.0*M)*(61.0+90.0*B+45.0*B*B);
	loa->y = lat * RAD2DEG;
	lon = L0*DEG2RAD+A/cos(Bf)-H/(6.0*cos(Bf))*(1.0+2.0*B+C)+I/(120.0*cos(Bf))*(5.0+28.0*B+24.0*B*B+6.0*C+8.0*B*C);
	loa->x = lon * RAD2DEG;
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
uint8_t extract_data(const uint8_t *data, size_t data_length) {
    const uint8_t header[] = {0xCC, 0x55, 0x00, 0x00};
    const uint8_t footer[] = {0xE0, 0x0E};
    const uint8_t *errordata = NULL;
    size_t header_length = sizeof(header);
    size_t footer_length = sizeof(footer);
    size_t expected_length = header_length + 33 + footer_length;
    uint8_t extracted_data[33];
    for (size_t i = 0; i <= data_length - expected_length; i++) 
    {
        if (memcmp(data + i, header, header_length) == 0 && memcmp(data + i + header_length + 32, footer, footer_length) == 0) 
        {
            memcpy(extracted_data, data + i + header_length, 33);
            return extracted_data;
        }else
        {
            return errordata;
        }
    }
    
}

// 读取十六进制数据并处理
void read_and_process_hex_data(int fd) 
{
    static uint8_t persistent_buffer[1024];
    static size_t buffer_length = 0;

    uint8_t temp_buffer[256];
    ssize_t bytes_read = read(fd, temp_buffer, sizeof(temp_buffer));
    if (bytes_read > 0) {
        if (buffer_length + bytes_read > sizeof(persistent_buffer)) {
            fprintf(stderr, "Buffer overflow\n");
            buffer_length = 0;
        } else {
            memcpy(persistent_buffer + buffer_length, temp_buffer, bytes_read);
            buffer_length += bytes_read;
        }
    }

    
    uint8_t extracted_data = extract_data(persistent_buffer, buffer_length);
    if (sizeof(extracted_data) == 33) {
        print_decimal_data(extracted_data);
        memmove(persistent_buffer, 0, buffer_length);
    
    }
}

// 获取用户输入
int get_user_input() {
    int choice;
    printf("Please select an option:\n");
    printf("1. Start positioning\n");
    printf("2. Stop positioning\n");
    printf("3. Start mapping\n");
    printf("4. Stop mapping\n");
    printf("Please select an option: ");
    scanf("%d", &choice);
    return choice;
}

int main() {
    int fd = open_serial_port("/dev/ttyUSB0", B115200);
    if (fd == -1) {
        return -1;
    }

    printf("Serial port opened successfully!\n");

    int choice = get_user_input();
    const uint8_t *hex_data = NULL;
    size_t hex_data_length = 0;

    switch (choice) {
        case 1:
            hex_data = (const uint8_t[]) {
                0xA0, 0x0A, 0x12, 0x73, 0x4D, 0x4E, 0x20, 0x6D, 0x43, 0x68, 0x61, 0x6E, 0x67, 0x65, 0x53, 0x74,
                0x61, 0x74, 0x65, 0x20, 0x33, 0x5F, 0xE0, 0x0E
            };
            hex_data_length = sizeof((const uint8_t[]) {
                0xA0, 0x0A, 0x12, 0x73, 0x4D, 0x4E, 0x20, 0x6D, 0x43, 0x68, 0x61, 0x6E, 0x67, 0x65, 0x53, 0x74,
                0x61, 0x74, 0x65, 0x20, 0x33, 0x5F, 0xE0, 0x0E
            });
            break;
        case 2:
            hex_data = (const uint8_t[]) {
                0xA0, 0x0A, 0x12, 0x73, 0x4D, 0x4E, 0x20, 0x6D, 0x43, 0x68, 0x61, 0x6E, 0x67, 0x65, 0x53, 0x74,
                0x61, 0x74, 0x65, 0x20, 0x34, 0x58, 0xE0, 0x0E
            };
            hex_data_length = sizeof((const uint8_t[]) {
                0xA0, 0x0A, 0x12, 0x73, 0x4D, 0x4E, 0x20, 0x6D, 0x43, 0x68, 0x61, 0x6E, 0x67, 0x65, 0x53, 0x74,
                0x61, 0x74, 0x65, 0x20, 0x34, 0x58, 0xE0, 0x0E
            });
            break;
        case 3:
            hex_data = (const uint8_t[]) {
                0xA0, 0x0A, 0x12, 0x73, 0x4D, 0x4E, 0x20, 0x6D, 0x43, 0x68, 0x61, 0x6E, 0x67, 0x65, 0x53, 0x74,
                0x61, 0x74, 0x65, 0x20, 0x31, 0x5D, 0xE0, 0x0E
            };
            hex_data_length = sizeof((const uint8_t[]) {
                0xA0, 0x0A, 0x12, 0x73, 0x4D, 0x4E, 0x20, 0x6D, 0x43, 0x68, 0x61, 0x6E, 0x67, 0x65, 0x53, 0x74,
                0x61, 0x74, 0x65, 0x20, 0x31, 0x5D, 0xE0, 0x0E
            });
            break;
        case 4:
            hex_data = (const uint8_t[]) {
                0xA0, 0x0A, 0x12, 0x73, 0x4D, 0x4E, 0x20, 0x6D, 0x43, 0x68, 0x61, 0x6E, 0x67, 0x65, 0x53, 0x74,
                0x61, 0x74, 0x65, 0x20, 0x32, 0x5E, 0xE0, 0x0E
            };
            hex_data_length = sizeof((const uint8_t[]) {
                0xA0, 0x0A, 0x12, 0x73, 0x4D, 0x4E, 0x20, 0x6D, 0x43, 0x68, 0x61, 0x6E, 0x67, 0x65, 0x53, 0x74,
                0x61, 0x74, 0x65, 0x20, 0x32, 0x5E, 0xE0, 0x0E
            });
            break;
        default:
            fprintf(stderr, "Invalid choice!\n");
            close(fd);
            return -1;
    }

    send_hex_data(fd, hex_data, hex_data_length);

    while (choice == 1 || choice == 3) {
        read_and_process_hex_data(fd);
        usleep(100000);
    }

    close(fd);

    return 0;
}