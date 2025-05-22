#include <stdio.h>
#include <proj.h>

int main() {
    // 定义输入UTM坐标参数
    double easting = 693515.675754;    // 东坐标（米）
    double northing =4048535.495559;  // 北坐标（米）
    int zone = 50;                // UTM区域号
    int is_north = 1;             // 是否北半球（1=北，0=南）

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
    return 0;
}