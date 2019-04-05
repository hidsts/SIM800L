# SIM800L


使用 SIM800L + STC90C516RD+ + VPS，构建一个通过网页随时随地收取验证码的系统。




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


