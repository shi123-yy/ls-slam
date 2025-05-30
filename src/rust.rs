use serialport::{DataBits, Parity, StopBits, SerialPortType, SerialPort};
use std::io::{self, Write, BufRead, BufReader, Read};
use std::fs::File;
use std::path;
use std::time::Duration;


// 串口配置结构体
struct SerialConfig 
{
    port_name: String,
    baud_rate: u32,
    data_bits: DataBits,
    parity: Parity,
    stop_bits: StopBits,
    timeout: Duration,
}

impl Default for SerialConfig 
{
    fn default() -> Self 
    {
        Self 
        {
            port_name: "/dev/ttyUSB0".to_string(),
            baud_rate: 115200,
            data_bits: DataBits::Eight,
            parity: Parity::None,
            stop_bits: StopBits::One,
            timeout: Duration::from_millis(1000),
        }
    }
}

// BCC校验计算
fn bcc_calc(data: &[u8]) -> u8 
{
    data.iter().fold(0u8, |acc, &x| acc ^ x)
}
// 发送无参数命令
fn send_no_param(port: &mut dyn SerialPort, cmd: &str) -> io::Result<()> {
    let base_cmd = format!("$HCNX,{}", cmd);
    let bcc = bcc_calc(base_cmd[1..].as_bytes());
    let full_cmd = format!("{}*{:02X}\r\n", base_cmd, bcc);
    
    port.write_all(full_cmd.as_bytes())?;
    println!("Sent: {}", full_cmd.trim());
    Ok(())
}

// 发送带参数命令
fn send_param(port: &mut dyn SerialPort, cmd: &str, params: &[&str]) -> io::Result<()> 
{
    let mut base_cmd = format!("$HCNX,{}", cmd);
    for param in params {
        base_cmd.push_str(&format!(",{}", param));
    }
    let bcc = bcc_calc(base_cmd[1..].as_bytes());
    let full_cmd = format!("{}*{:02X}\r\n", base_cmd, bcc);
    
    port.write_all(full_cmd.as_bytes())?;
    println!("Sent: {}", full_cmd.trim());
    Ok(())
}


// 处理配置文件
fn process_config(port: &mut dyn SerialPort, path: &str) -> io::Result<()> { // 修正函数签名
    let file = File::open(path)?;
    let reader = BufReader::new(file);

    for (line_num, line) in reader.lines().enumerate() {
        let line = line?;
        
        // 跳过空行和注释
        if line.trim().is_empty() || line.starts_with('#') {
            continue;
        }

        let parts: Vec<&str> = line.split(',').collect(); // 修复变量名错误
        
        if parts.is_empty() {
            eprintln!("Line {}: Empty command", line_num + 1);
            continue;
        }

        match parts[0].trim() {
            // 带参数命令（需要至少1个参数）
            "SETNET" | "NETSTATE" | "LOCATION" | "SET_WORKMODE" | "SET_CONFIG" 
            | "SET_TRACKER_INSTALL_PARAM" | "SET_ANGLE_INSTALL_PARAM" 
            | "SET_SINGLE_BEIDOU_MODE" | "SET_VEH_CTRLPOINT_OUTPUT_PARAM" 
            | "SET_VEH_WHEEL_BASE" | "SETAPN" => {
                if parts.len() < 2 {
                    eprintln!("Line {}: Missing parameters for {}", line_num + 1, parts[0]);
                    continue;
                }
                send_param(port, parts[0], &parts[1..])?;
            },
            // 无参数命令
            "GETNET" | "GETAPN" | "GETICCID" | "CONT_CRC_ERROR" | "GET_PN_INFO" 
            | "CLEAN_EPH" | "GET_DEVICE_MSG" | "SET_HEADING_INVERSION" 
            | "GET_VEH_CTRLPOINT_OUTPUT_PARAM" => {
                send_no_param(port, parts[0])?;
            },
            _ => eprintln!("Line {}: Unknown command: {}", line_num + 1, parts[0]),
        }
        
        // 确保命令立即发送
        port.flush()?;
    }
    Ok(())
}

// 用户交互菜单
fn print_menu() {
    let menu_items = [
        "设置网络参数[1]",
        "获取网络参数[2]",
        "上传网络状态[3]",
        "设置APN参数[4]",
        "获取APN参数[5]",
        "获取流量卡参数[6]",
        "接收命令时CRC校验码错误[7]",
        "查询控制器PN号[8]",
        "清除星历[9]",
        "关闭支持的卫星[10]",
        "获取设备信息[11]",
        "设置工作模式[12]",
        "设置电台参数[13]",
        "设置拖拉机安装参数[14]",
        "设置姿态角安装误差参数[15]",
        "设置单北斗模式[16]",
        "设置航向取反[17]",
        "设置是否打开车辆控制点坐标输出即车辆类型[18]",
        "获取是否打开车辆控制点坐标输出即车辆类型[19]",
        "设置车辆前后轴距离[20]",
    ];

    for (i, item) in menu_items.iter().enumerate() {
        println!("{}: {}", i + 1, item);
    }
}
// 初始化串口
fn serial_init(config: SerialConfig) -> Result<Box<dyn SerialPort>, serialport::Error> 
{
    serialport::new(&config.port_name, config.baud_rate)
        .data_bits(config.data_bits)
        .parity(config.parity)
        .stop_bits(config.stop_bits)
        .timeout(config.timeout)
        .open()
}

fn load_command_map(path: &str) -> io::Result<Vec<(String, Option<Vec<String>>)>>
{
    let file = File::open(path)?;
    let reader = BufReader::new(file);
    let mut command_map = Vec::new();   
    for (line_num, line) in reader.lines().enumerate() {
        let line = line?;
        // 跳过空行和注释
        if line.trim().is_empty() || line.starts_with('#') {
            continue;
        }

        let parts: Vec<_> = line.split(',').map(|s| s.trim()).collect();
        if parts.is_empty() {
            return Err(io::Error::new(
                io::ErrorKind::InvalidData,
                format!("Line {}: Empty command", line_num + 1),
            ));
        }

        let cmd = parts[0].to_string();
        let params = if parts.len() > 1 {
            Some(parts[1..].iter().map(|&s| s.to_string()).collect())
        } else {
            None
        };

        command_map.push((cmd, params));
    }
    Ok(command_map)
}

fn main() -> Result<(), Box<dyn std::error::Error>> 
{
    
    // 初始化串口
    let config = SerialConfig::default();
    let port =serial_init(config)?;

    // process_config(&mut *port,"./config/config.txt")?; 

    

    // 创建读写分离
    let mut read = port.try_clone()?;
    let mut writer = port;

    // 启动读取线程
    std::thread::spawn(move || {
        let mut buf = [0; 256];
        loop {
            match read.read(&mut buf) {
                Ok(n) => {
                    let received = String::from_utf8_lossy(&buf[..n]);
                    print!("Received: {}", received);
                }
                Err(ref e) if e.kind() == io::ErrorKind::TimedOut => continue,
                Err(e) => eprintln!("Read error: {}", e),
            }
        }
    });
    // process_config(&mut *port,"./config/config.txt")?; 

    // 主线程处理输入
    let command_map = load_command_map("./config/config.txt")?;
    // 用户交互
    print_menu();
    println!("请输入命令编号 (1-20，Ctrl+C退出):");
    let stdin = io::stdin();
    for line in stdin.lock().lines() {
        let input = line?;
        match input.trim().parse::<usize>() 
        {
            Ok(n) if n >= 1 && n <= 20 => 
            {
                let (cmd, params) = &command_map[n-1];
                match params {
                    Some(p) => 
                    {
                        let str_params: Vec<&str> = p.iter().map(|s| s.as_str()).collect();
                        send_param(&mut *writer, cmd, &str_params)?
                    },
                    None => send_no_param(&mut *writer, cmd)?,
                }
            },
            Ok(n) => println!("无效编号: {}，请输入1-20之间的数字", n),
            Err(_) => println!("无效输入，请输入数字"), // 添加错误处理分支
            // ... 保持错误处理不变 ...
        }
        println!("请继续输入命令编号 (1-20):");
    }

    Ok(())
}