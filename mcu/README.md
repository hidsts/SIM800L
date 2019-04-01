# SIM800L

##硬件：

MODE: SIM800L         * 1
MCU:  STC90C516RD+    * 1



##工作流程：

SIM800L 和 MCU 通过串口通信，程序启动时会尝试连接SIM800L模块，然后进入等待收短信流程。

SIM800L收到新短信时，通过串口将内容发送至MCU，MCU尝试连接TCP服务器将数据送出，然后回到等待流程。



##已知问题：

程序只能接收70个汉字以内的单条短信，超过就会宕机。同时收到多条短信也可能会。因为硬件条件限制（MCU只有1248字节内存），准备更改 SIM800L 和 MCU 工作模式来解决。

