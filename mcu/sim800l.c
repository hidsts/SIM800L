#include <reg51.h>  
#include <string.h>

#define LENGTH 400		// �յ�һ�������Ŵ��350�ֽ�

unsigned char code TC_CONNECT[] = "AT";					// ������������
unsigned char code TC_RUNMODE[] = "AT+CNMI=1,2,0,0,0";	// ��������ģʽ

unsigned char code TC_TCP_CHECKSIM[] = "AT+CPIN?";		// 1.���SIM״̬
unsigned char code TC_TCP_CHECKNET[] = "AT+CSQ";			// 2.��������ź�ǿ��
unsigned char code TC_TCP_CHECKREG[] = "AT+CREG?";		// 3.�������ע��״̬
unsigned char code TC_TCP_CHECKGPRS[] = "AT+CGATT?"; 		// 4.���GPRS����״̬
unsigned char code TC_TCP_SETAPN[] = "AT+CSTT=\"CMNET\"";// 5.����APN
unsigned char code TC_TCP_GETLINK[] = "AT+CIICR";		// 6.����������·
unsigned char code TC_TCP_GETIP[] = "AT+CIFSR";			// ��ñ���IP��ַ
unsigned char code TC_TCP_TOSERVER[] = "AT+CIPSTART=\"TCP\",\"193.112.94.54\",\"9001\"";
unsigned char code TC_TCP_SENDTEXT[] = "AT+CIPSEND";		// ���;�������
unsigned char code TC_TCP_CLOSE[] = "AT+CIPCLOSE";		// �����ر�����
unsigned char code TC_TCP_SHUT[] = "AT+CIPSHUT";			// �ر�GPRSͨ��


unsigned char xdata r_datas[LENGTH] = {0};	 					// ����MCU���������
unsigned char xdata t_datas[LENGTH] = {0};	 					// ����TCP������������
unsigned int MAX=0;
unsigned int index=0;								// ������������

//--����ȫ�ֺ���--//
void UsartConfiguration();				// ���ڲ�����
void Delay10ms(unsigned int c);   		// ��� 0us		# �о�Ҫ*20 ����1000ms����20s
void send_byte(unsigned char ch);		// ����һ���ֽ� 
void print_r_datas();						// �����ã����r_datas����
void print_t_datas();						// �����ã����t_datas����
void clean_r_datas();						// data1��������

void sim800l_connect();					// ����Ӧ����������SIM800Lģ��
void sim800l_set_runmode();				// ����ģ����յ��¶���ֱ��ת����MCU
void sim800l_wait_sms();				// ��ѯ����

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
* �� �� ��         : main
* ��������		   : ������
* ��    ��         : ��
* ��    ��         : ��
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


	while(1);		// �����ã���ֹ��ѭ���������������
	
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
		
		// ����ӳ�ʱ������㹻����Ȼ�����յ������ݣ����ţ�������
		// �������ж��㹻��ʱ��ȥ�������ݷ�������
		p = strstr(r_datas,"CMT");
		if(p != NULL)
		{
			P0 = 0xa5;	 // 1010 0101
			send_byte(0x53);   // S
//			print_r_datas();
			send_byte(0x53);

			// ���յ������ݻ���
			strcpy(t_datas,r_datas);

			// ��ս��ջ�������
			clean_r_datas();

			// ���ŷ��͵�TCP������
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
//	clean_r_datas();   // ���ص���CONNETC OK������һ���ж�û��Ӱ�죬���Բ�����գ���������ٶ�

	tcp_9_sendtext();
//	clean_r_datas();	// �������жϣ�ע�͵���������ٶ�

	tcp_10_close();
//	clean_r_datas();   	// �������жϣ�ע�͵���������ٶ�

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

	 	//��ȷ���أ�
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

		//��ȷ���أ�
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

		//��ȷ���أ�
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

		//��ȷ���أ�
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

//	while(1)   // ���ó���һ�ξͻ����
//	{
		for(i = 0; i < sizeof(TC_TCP_SETAPN)-1; i++)
	 	{
			send_byte(TC_TCP_SETAPN[i]);
		}
		send_byte(0x0d);
		send_byte(0x0a);
		Delay10ms(5);

		P0 = 0x40;	 // 0100 0000


		//��ȷ����
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

//	while(1)  // ���ó���һ�ξͻ����
//	{
		for(i = 0; i < sizeof(TC_TCP_GETLINK)-1; i++)
	 	{
			send_byte(TC_TCP_GETLINK[i]);
		}
		send_byte(0x0d);
		send_byte(0x0a);
		Delay10ms(5);

		P0 = 0x80;	 // 1000 0000


		//��ȷ����
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


		//��ȷ���أ�
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


		//��ȷ���أ�
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
		Delay10ms(5); // �ڵ��Ե�ʱ����Ҫ���ӳټӴ��򴮿ڷ���'>'�ַ������Ż���TCP��������������
						// ����ģ���ʱ������ʵ���С
		P0 = 0x30;	 // 0011 0000

		//	��ȷ���أ�
		//	>

		//	Ȼ����������
		//  ��������������1A

		//	���ͳɹ����أ�
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
		
//		Delay10ms(10);		// ���ﲻ���ӳ١��ӳٻᵼ���ղ���SEND OK�źţ�ԭ����	

		P0 = 0xc0;	 // 1100 0000

	
		g = strstr(r_datas,"SEND OK");
		if(g != NULL)
		{
			P0 = 0xe0;	 // 1110 0000

			// ���t_datas
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

		// ���ͳɹ����ص����ַ��� "CLOSE OK"
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
	SCON=0X50;			//����Ϊ������ʽ1
	TMOD=0X20;			//���ü�����������ʽ2
	PCON=0X80;			//�����ʼӱ�
	TH1=0XF3;		    //��������ʼֵ���ã�ע�Ⲩ������4800��
	TL1=0XF3;
	ES=1;						//�򿪽����ж�
	EA=1;						//�����ж�
	TR1=1;					    //�򿪼�����
}


void Delay10ms(unsigned int c)   //��� 0us
{
    unsigned char a, b;

	//--c�Ѿ��ڴ��ݹ�����ʱ���Ѿ���ֵ�ˣ�������for����һ��Ͳ��ø�ֵ��--//
    for (;c>0;c--)
	{
		for (b=38;b>0;b--)
		{
			for (a=130;a>0;a--);
		}          
	}       
}