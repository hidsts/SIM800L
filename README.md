# SIM800L


使用 SIM800L + STC90C516RD+ + VPS构建的短信转发系统。




## 项目介绍：  
  
  
本项目主要使用 STC90C516RD+ MCU做控制。使用 SIM800L GSM MODULE收发短信，联网等。

MCU 和 GSM MODULE 通过串口进行通信。GSM MODULE 收到新短信时，会先存入SIM卡。

MCU 采用轮询的方式查收短信（PDU编码格式）。发现新短信后，通过 GSM MODULE 使用TCP方式提交到 VPS。

TCP服务端使用 Python 编写，TCP 服务端收到短信之后分辨编码方式。

解码后通过 requests 模块将短信提交到 Web 服务器。

Web 服务器使用 SpringBoot 快速开发，数据库采用 MySQL。



## 已知问题：  
  
~~程序只能接收70个汉字以内的单条短信，超过就会宕机。同时收到多条短信也可能会。因为硬件条件限制（MCU只有1248字节内存），准备更改 SIM800L 和 MCU 工作模式来解决。~~
  
v2.0更改工作模式，GSM MODULE收到短信之后先存入 SIM 卡，而不是直接通过串口发送到 MCU。所以没有这个问题



## 更新记录：  
  
v2.0.2    
1.优化流程，确认收到短信后先使用AT+CIPSHUT关闭PDP上下文再连接服务器，完成之后也关闭一次PDP上下文（之前虽然由服务端关闭了TCP链接，但PDP上下文仍在）。
2.删除向模块发AT命令之前清除RX_BUFFER函数，改为重置索引，提高反馈判断成功率。
3.波特率由19200提高至115200，机器周期由12T改成6T，提高运行速度。
4.发短信之前关闭中断，发完之后再开启，极大的提高发送效率。
5.添加查询IP错误超过一定次数后置运行状态为SHUT。
6.修改串口通信参数中的错误。
  
v2.0.1      
1.读短信遍历由递减修改为递增，提高命中率，提升运行效率。  
2.增加格式化短信函数，除去PDU之外的字符串。  
    
v2.0.0  
1.重构代码，更清晰的运行流程
      