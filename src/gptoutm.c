#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proj.h>
#include <math.h>
#define M_PI 3.14159265358979323846
#define WGS84_A 6378137.0           // WGS-84 椭球体长半轴
#define WGS84_F 1.0 / 298.257223563 // WGS-84 椭球体扁率
#define UTM_SCALE_FACTOR 0.9996     // UTM 比例因子
#define UTM_EAST_OFFSET 500000.0    // UTM 东偏移量
typedef struct
{
    /* data */
    // double lat;
    // double lon;

    double x;
    double y;
}Pos;

int calculate_utm_zone(double longitude) {
    return (int)((longitude + 180.0) / 6.0) + 1;
}
    
// 将 WGS-84 经纬度转换为 UTM 坐标
Pos latlon_to_utm(double lat, double lon)
{
    double x,y;
    double a = WGS84_A;
    double f = WGS84_F;
    double k0 = UTM_SCALE_FACTOR;
    double east_offset = UTM_EAST_OFFSET;
    Pos pos={0};
    // 计算椭球体参数
    double b = a * (1.0 - f); // 短半轴
    double e_sq = (a * a - b * b) / (a * a); // 第一偏心率的平方
    double e_prime_sq = (a * a - b * b) / (b * b); // 第二偏心率的平方

    // 将经纬度转换为弧度
    double lat_rad = lat * M_PI / 180.0;
    double lon_rad = lon * M_PI / 180.0;

    // 计算 UTM 带号和中央经线
    int zone = calculate_utm_zone(lon);
    double lon_origin = ((zone - 1) * 6 - 180 + 3)*M_PI/180.0; // 中央经线

    // 计算辅助参数
    double N = a / sqrt(1.0 - e_sq * sin(lat_rad) * sin(lat_rad));
    double T = tan(lat_rad) * tan(lat_rad);
    double C = e_prime_sq * cos(lat_rad) * cos(lat_rad);
    double A = cos(lat_rad) * (lon_rad - lon_origin);

    // 计算 M（子午线弧长）
    double M = a * ((1.0 - e_sq / 4.0 - 3.0 * e_sq * e_sq / 64.0 - 5.0 * e_sq * e_sq * e_sq / 256.0) * lat_rad
               - (3.0 * e_sq / 8.0 + 3.0 * e_sq * e_sq / 32.0 + 45.0 * e_sq * e_sq * e_sq / 1024.0) * sin(2.0 * lat_rad)
               + (15.0 * e_sq * e_sq / 256.0 + 45.0 * e_sq * e_sq * e_sq / 1024.0) * sin(4.0 * lat_rad)
               - (35.0 * e_sq * e_sq * e_sq / 3072.0) * sin(6.0 * lat_rad));

    // 计算东偏移量和北偏移量
    x = k0 * N * (A + (1.0 - T + C) * A * A * A / 6.0
               + (5.0 - 18.0 * T + T * T + 72.0 * C - 58.0 * e_prime_sq) * A * A * A * A * A / 120.0)
               + east_offset;

    y = k0 * (M + N * tan(lat_rad) * (A * A / 2.0
                    + (5.0 - T + 9.0 * C + 4.0 * C * C) * A * A * A * A / 24.0
                    + (61.0 - 58.0 * T + T * T + 600.0 * C - 330.0 * e_prime_sq) * A * A * A * A * A * A / 720.0));

    // 如果纬度在南半球，调整北偏移量
    if (lat < 0) {
        y += 10000000.0;
    }
    // pos.lat = lat;
    // pos.lon = lon;
    pos.x = x;
    pos.y = y;
    return pos;
}

    
int main() 
{
    FILE *fp;
    char buffer[1024];
    Pos pos;
    fp = fopen("./data/gga.txt", "r");
    if (fp != NULL)
    {
        printf("open gga.txt success\n");
    }
        
    FILE *fileout;
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        // buffer[strcspn(buffer, "\r\n")] = '\0';  // 去除行尾符
        if (strncmp(buffer, "#INSPVAA", 8) == 0)
        {
            /* code */
            // printf("INSPVAA: %s\n",buffer);
            char *data[20];
            char *token = strtok(buffer, ",");
            int i = 0;
            while (token != NULL)
            {
                /* code */
                data[i] = token;
                // printf("token %d: %s\n",i,token);
                token = strtok(NULL, ",");
                i++;
                if (i > 15)
                {
                    // printf("经纬度：%s,%s\n",data[11],data[12]);
                    double lat = atof(data[11]);
                    double lon = atof(data[12]);
                    pos = latlon_to_utm(lat,lon);
                    printf("UTM X: %f, UTM Y: %f\n", pos.x, pos.y);
                    fileout = fopen("./data/utm.txt","a");
                    if (fileout != NULL) {
                        fprintf(fileout, "%f, %f\n", pos.x, pos.y); 
                        fclose(fileout);
                    }
                    break;
                    /* code */
                }
                
            }
            // printf("size: %d\n",sizeof(buffer));
            // pos = gpsToUTM(48.1167,11.6833);
        }
        // memchr(buffer,',',10);
        memset(buffer, 0, sizeof(buffer));
        /* code */
    }
    fclose(fp);
    return 0;

}