#include <reg51.h>  
#include <string.h>

#define LENGTH 400		// 收到一条最大短信大概350字节

unsigned char code TC_CONNECT[] = "AT";					// 发送内容连接
unsigned char code TC_RUNMODE[] = "AT+CNMI=1,2,0,0,0";	// 发送内容模式

unsigned char code TC_TCP_CHECKSIM[] = "AT+CPIN?";		// 1.检查SIM状态
unsigned char code TC_TCP_CHECKNET[] = "AT+CSQ";			// 2.检查网络信号强度
unsigned char code TC_TCP_CHECKREG[] = "AT+CREG?";		// 3.检查网络注册状态
unsigned char code TC_TCP_CHECKGPRS[] = "AT+CGATT?"; 		// 4.检查GPRS附着状态
unsigned char code TC_TCP_SETAPN[] = "AT+CSTT=\"CMNET\"";// 5.设置APN
unsigned char code TC_TCP_GETLINK[] = "AT+CIICR";		// 6.建立无线链路
unsigned char code TC_TCP_GETIP[] = "AT+CIFSR";			// 获得本地IP地址
unsigned char code TC_TCP_TOSERVER[] = "AT+CIPSTART=\"TCP\",\"193.112.94.54\",\"9001\"";
unsigned char code TC_TCP_SENDTEXT[] = "AT+CIPSEND";		// 发送具体数据
unsigned char code TC_TCP_CLOSE[] = "AT+CIPCLOSE";		// 主动关闭链接
unsigned char code TC_TCP_SHUT[] = "AT+CIPSHUT";			// 关闭GPRS通信


unsigned char xdata r_datas[LENGTH] = {0};	 					// 缓存MCU传入的数据
unsigned char xdata t_datas[LENGTH] = {0};	 					// 传到TCP服务器的数据
unsigned int MAX=0;
unsigned int index=0;								// 缓存数组索引

//--声明全局函数--//
void UsartConfiguration();				// 串口参数配
void Delay10ms(unsigned int c);   		// 误差 0us		# 感觉要*20 就是1000ms等于20s
void send_byte(unsigned char ch);		// 发送一个字节 
void print_r_datas();						// 调试用，输出r_datas数据
void print_t_datas();						// 调试用，输出t_datas数据
void clean_r_datas();						// data1数组清零

void sim800l_connect();					// 自适应波特率连接SIM800L模块
void sim800l_set_runmode();				// 设置模块接收到新短信直接转发至MCU
void sim800l_wait_sms();				// 轮询短信

void tcp();
void tcp_1_checksim();
void tcp_2_checknet();
void tcp_3_checkreg();
void tcp_4_checkgprs();
void tcp_5_setapn();
void tcp_6_getlink();
void tcp_7_getip();
void tcp_8_toserver();
void tcp_9_sendtext();
void tcp_10_close();
void tcp_11_shut();


/*******************************************************************************
* 函 数 名         : main
* 函数功能		   : 主函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/

void main()
{
	UsartConfiguration();

	P0 = 0x01;	 // 0000 0001

	SBUF = 's';

	sim800l_connect();

	clean_r_datas();

	sim800l_set_runmode();

	clean_r_datas();

	sim800l_wait_sms();



	send_byte('e');


	while(1);		// 调试用，防止死循环看不出代码差异
	
}


void send_byte(unsigned char ch)
{
	while(TI!=1);
	TI = 0;
	SBUF = ch;
}



void print_r_datas()
{
	int i;
	for(i = 0; i < strlen(r_datas); i++)
	{
		send_byte(r_datas[i]);	
	}
}

void print_t_datas()
{
	int i;
	for(i = 0; i < strlen(t_datas); i++)
	{
		send_byte(t_datas[i]);	
	}
}

void clean_r_datas()
{
	memset(r_datas,0,sizeof(r_datas));
	index = 0;
}


void sim800l_connect()
{
	int i;
	unsigned char *p;

	while(1)
	{
		for(i = 0; i < sizeof(TC_CONNECT)-1; i++)
		{
			send_byte(TC_CONNECT[i]);	
		}
		send_byte(0x0d);
		send_byte(0x0a);
		Delay10ms(1);

		p = strstr(r_datas,"OK");
		if(p != NULL)
		{
			break;
		}
	}
}


void sim800l_set_runmode()
{
	int i;
	unsigned char *p;

	while(1)
	{
		for(i = 0; i < sizeof(TC_RUNMODE)-1; i++)
	 	{
			send_byte(TC_RUNMODE[i]);
		}
		send_byte(0x0d);
		send_byte(0x0a);
		Delay10ms(1);

		p = strstr(r_datas,"OK");
		if(p != NULL)
		{
			break;

		}
	}
}


void sim800l_wait_sms()
{
	char *p;
	while(1)
	{
	 	// do sms;
		P0 = 0x02;	 // 0000 0010

		send_byte(0x57);		// W
		send_byte(0x41);		// A
		send_byte(0x49);		// I
		send_byte(0x54); 		// T
		send_byte(0x0d);
		send_byte(0x0a);
		Delay10ms(10);
		
		// 这个延迟时间必须足够，不然串口收到的数据（短信）不完整
		// 给串口中断足够的时间去接收数据放入数组
		p = strstr(r_datas,"CMT");
		if(p != NULL)
		{
			P0 = 0xa5;	 // 1010 0101
			send_byte(0x53);   // S
//			print_r_datas();
			send_byte(0x53);

			// 将收到的数据缓存
			strcpy(t_datas,r_datas);

			// 清空接收缓存数组
			clean_r_datas();

			// 短信发送到TCP服务器
			tcp();

			P0 = 0x0f;	 // 0000 1111


		}
		clean_r_datas();
	}

}

void tcp()
{
	tcp_1_checksim();
	clean_r_datas();

	tcp_2_checknet();
	clean_r_datas();
	tcp_3_checkreg();
	clean_r_datas();

	tcp_4_checkgprs();
	clean_r_datas();

	tcp_5_setapn();
	clean_r_datas();

	tcp_6_getlink();
	clean_r_datas();

	tcp_7_getip();
	clean_r_datas();

	tcp_8_toserver();
//	clean_r_datas();   // 返回的是CONNETC OK，对下一步判断没有影响，可以不用清空，提高运行速度

	tcp_9_sendtext();
//	clean_r_datas();	// 后面无判断，注释掉提高运行速度

	tcp_10_close();
//	clean_r_datas();   	// 后面无判断，注释掉提高运行速度

	tcp_11_shut();
	clean_r_datas();

}

void tcp_1_checksim()
{
 	int i;
	unsigned char *p;

	while(1)
	{
		for(i = 0; i < sizeof(TC_TCP_CHECKSIM)-1; i++)
	 	{
			send_byte(TC_TCP_CHECKSIM[i]);
		}
		send_byte(0x0d);
		send_byte(0x0a);
		Delay10ms(5);

	 	//正确返回：
		//+CPIN: READY

		//OK
		p = strstr(r_datas,"OK");
		if(p != NULL)
		{
			P0 = 0x04;	 // 0000 0100

			break;
		}
	}

}

void tcp_2_checknet()
{
 	int i;
	unsigned char *p;

	while(1)
	{
		for(i = 0; i < sizeof(TC_TCP_CHECKNET)-1; i++)
	 	{
			send_byte(TC_TCP_CHECKNET[i]);
		}
		send_byte(0x0d);
		send_byte(0x0a);
		Delay10ms(5);

		//正确返回：
		//+CSQ: 20,0

		//OK
		p = strstr(r_datas,"OK");
		if(p != NULL)
		{
			P0 = 0x08;	 // 0000 1000

			break;
		}
	}

}

void tcp_3_checkreg()
{
 	int i;
	unsigned char *p;

	while(1)
	{
		for(i = 0; i < sizeof(TC_TCP_CHECKREG)-1; i++)
	 	{
			send_byte(TC_TCP_CHECKREG[i]);
		}
		send_byte(0x0d);
		send_byte(0x0a);
		Delay10ms(5);

		//正确返回：
		//+CREG: 0,1

		//OK
		p = strstr(r_datas,"OK");
		if(p != NULL)
		{
			P0 = 0x10;	 // 0001 0000

			break;
		}
	}

}

void tcp_4_checkgprs()
{
 	int i;
	unsigned char *p;

	while(1)
	{
		for(i = 0; i < sizeof(TC_TCP_CHECKGPRS)-1; i++)
	 	{
			send_byte(TC_TCP_CHECKGPRS[i]);
		}
		send_byte(0x0d);
		send_byte(0x0a);
		Delay10ms(5);

		//正确返回：
		//+CGATT: 1

		//OK
		p = strstr(r_datas,"OK");
		if(p != NULL)
		{
			P0 = 0x20;	 // 0010 0000

			break;
		}
	}

}

void tcp_5_setapn()
{
 	int i;
//	unsigned char *p;

//	while(1)   // 设置超过一次就会错误
//	{
		for(i = 0; i < sizeof(TC_TCP_SETAPN)-1; i++)
	 	{
			send_byte(TC_TCP_SETAPN[i]);
		}
		send_byte(0x0d);
		send_byte(0x0a);
		Delay10ms(5);

		P0 = 0x40;	 // 0100 0000


		//正确返回
		//OK
//		p = strstr(r_datas,"OK");
//		if(p != NULL)
//		{
//			break;
//		}
//	}

}

void tcp_6_getlink()
{
 	int i;
//	unsigned char *p;

//	while(1)  // 设置超过一次就会错误
//	{
		for(i = 0; i < sizeof(TC_TCP_GETLINK)-1; i++)
	 	{
			send_byte(TC_TCP_GETLINK[i]);
		}
		send_byte(0x0d);
		send_byte(0x0a);
		Delay10ms(5);

		P0 = 0x80;	 // 1000 0000


		//正确返回
		//OK
//		p = strstr(r_datas,"OK");
//		if(p != NULL)
//		{
//			break;
//		}
//	}

}

void tcp_7_getip()
{
 	int i;
//	unsigned char *p;

//	while(1)  
//	{
		for(i = 0; i < sizeof(TC_TCP_GETIP)-1; i++)
	 	{
			send_byte(TC_TCP_GETIP[i]);
		}
		send_byte(0x0d);
		send_byte(0x0a);
		Delay10ms(5);


		P0 = 0x03;	 // 0000 0011


		//正确返回：
		//10.42.6.249
//		p = strstr(r_datas,".");
//		if(p != NULL)
//		{
//			break;
//		}
//	}

}

void tcp_8_toserver()
{
 	int i;
	unsigned char *p;

	while(1)
	{
		for(i = 0; i < sizeof(TC_TCP_TOSERVER)-1; i++)
	 	{
			send_byte(TC_TCP_TOSERVER[i]);
		}
		send_byte(0x0d);
		send_byte(0x0a);
		Delay10ms(10);

		P0 = 0x0c;	 // 0000 1100

		send_byte(0x30); // 0
		send_byte(0x0d);
		send_byte(0x0a);
		print_r_datas();
		send_byte(0x0d);
		send_byte(0x0a);
		send_byte(0x30); // 0


		//正确返回：
		//OK

		//CONNECT OK
		p = strstr(r_datas,"CONNECT OK");
		if(p != NULL)
		{
			break;
		}
	}

}

void tcp_9_sendtext()
{
 	int i;
	unsigned char *p;
	unsigned char *g;
	int j;

	while(1)
	{
		for(i = 0; i < sizeof(TC_TCP_SENDTEXT)-1; i++)
	 	{
			send_byte(TC_TCP_SENDTEXT[i]);
		}
		send_byte(0x0d);
		send_byte(0x0a);
		Delay10ms(5); // 在调试的时候需要把延迟加大，向串口发送'>'字符串，才会向TCP服务器发送数据
						// 连接模块的时候可以适当调小
		P0 = 0x30;	 // 0011 0000

		//	正确返回：
		//	>

		//	然后输入内容
		//  最后输入结束符：1A

		//	发送成功返回：
		//	SEND OK
		//
		//	CLOSED

		p = strstr(r_datas,">");
		if(p != NULL)
		{
			break;							  
		}
	}

	while(1)
	{
		for(j = 13;j < strlen(t_datas); j++)
		{
			send_byte(t_datas[j]);
		}
		send_byte(0x0d);
		send_byte(0x0a);
		send_byte(0x1a);
		
//		Delay10ms(10);		// 这里不能延迟。延迟会导致收不到SEND OK信号，原因不明	

		P0 = 0xc0;	 // 1100 0000

	
		g = strstr(r_datas,"SEND OK");
		if(g != NULL)
		{
			P0 = 0xe0;	 // 1110 0000

			// 清空t_datas
			memset(t_datas,0,sizeof(t_datas)*LENGTH);
			break;
		}
	}
}

void tcp_10_close()
{
 	int i;
	unsigned char *p;

//	while(1)
//	{
		for(i = 0; i < sizeof(TC_TCP_CLOSE)-1; i++)
	 	{
			send_byte(TC_TCP_CLOSE[i]);
		}
		send_byte(0x0d);
		send_byte(0x0a);
		Delay10ms(5);

		P0 = 0x07;	 // 0000 0111

		// 发送成功返回的是字符串 "CLOSE OK"
		p = strstr(r_datas,"OK");
		if(p != NULL)
		{
//			break;
		}
//	}

}

void tcp_11_shut()
{
	int i;
	unsigned char *p;
	
	for(i = 0; i < sizeof(TC_TCP_SHUT)-1; i++)
	{
		send_byte(TC_TCP_SHUT[i]);
	}
	send_byte(0x0d);
	send_byte(0x0a);
	Delay10ms(5);	

	p = strstr(r_datas,"SHUT OK");

	if(p != NULL)
	{
		//do something
	}

	P0 = 0x38;	 // 0011 1000

}




void serial() interrupt 4
{
 	if(RI == 1)
	{
		RI = 0;
//		index = 0;
		r_datas[index++] = SBUF;
		if(index > MAX)
		{
			MAX = index;
		}
		if(index > sizeof(r_datas))
		{
			index = 0;
		}

	}
}



void UsartConfiguration()
{
	SCON=0X50;			//设置为工作方式1
	TMOD=0X20;			//设置计数器工作方式2
	PCON=0X80;			//波特率加倍
	TH1=0XF3;		    //计数器初始值设置，注意波特率是4800的
	TL1=0XF3;
	ES=1;						//打开接收中断
	EA=1;						//打开总中断
	TR1=1;					    //打开计数器
}


void Delay10ms(unsigned int c)   //误差 0us
{
    unsigned char a, b;

	//--c已经在传递过来的时候已经赋值了，所以在for语句第一句就不用赋值了--//
    for (;c>0;c--)
	{
		for (b=38;b>0;b--)
		{
			for (a=130;a>0;a--);
		}          
	}       
}