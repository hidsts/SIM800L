#include <reg51.h>
#include <string.h>


#define START       0x00
#define AIRPLANE    0x01
#define MOBILE      0x02
#define REARY       0x03
#define ONLINE      0x04
#define SIM         0x05
#define REG         0x06
#define SIGNAL      0x07
#define SHUT        0x08
#define NEWSMS      0x09
#define APN         0x10
#define GPRS        0x11
#define IP          0x12
#define SERVER      0x13 

#define RX_LEN      60
#define TX_LEN      400 
#define TOCHAR(x) (x + '0')

char    code    CMD_AIRPLANE[]    =   "AT+CFUN=0\r\n";
char    code    CMD_MOBILE[]      =   "AT+CFUN=1\r\n";
char    code    CMD_AT[]          =   "AT\r\n";
char    code    CMD_SIM[]         =   "AT+CPIN?\r\n";
char    code    CMD_REG[]         =   "AT+CREG?\r\n";
char    code    CMD_SIGNAL[]      =   "AT+CSQ\r\n";
char    code    CMD_SHUT[]        =   "AT+CIPSHUT\r\n";
char    code    CMD_SHUTGPRS[]    =   "AT+CGATT=0\r\n";
char    code    CMD_CKSMS[]       =   "AT+CPMS?\r\n";
char    code    CMD_GTSMS[]       =   "AT+CMGR=";
char    code    CMD_DLSMS[]       =   "AT+CMGD=";
char    code    CMD_APN[]         =   "AT+CSTT=\"CMNET\"\r\n";
char    code    CMD_GPRS[]        =   "AT+CIICR\r\n";
char    code    CMD_IP[]          =   "AT+CIFSR\r\n";
char    code    CMD_SERVER[]      =   "AT+CIPSTART=\"TCP\",\"193.112.94.54\",\"9001\"\r\n";
char    code    CMD_SEND[]        =   "AT+CIPSEND\r\n";

unsigned    char    data    RX_BUFFER[RX_LEN]   =   {0};
unsigned    int     data    RX_INDEX            =   0; 
unsigned    char    xdata   TX_BUFFER[TX_LEN]   =   {0};
unsigned    int     data    TX_INDEX            =   0; 
unsigned    int     CHANGE_BUFFER               =   0;
unsigned    char    STATUS;
unsigned    char    SMS_CNT[2] = {0};
int i;

void RUN();
void SEND_BYTE(unsigned char c);
void SEND_STRING(unsigned char *p);
void CLEAN_RX_BUFFER();
void CLEAN_TX_BUFFER();
void Delay1ms(unsigned int i);
void SERIAL();
void UART();
void DEBUG();
void FORMAT_TX_BUFFER();



void main()
{
    P0 = 0X00;
    STATUS = START;
    while(1)
    {
       RUN(); 
    }
}


void RUN()
{
    if(STATUS == START)
    {
        //  INIT();             初始化串口等设置
        //  YES :   READY 
        //  NO  :

        UART();
        STATUS = REARY;
    }

    else if(STATUS == AIRPLANE)
    {
        //  "AT+CFUN=0" ;        开启飞行模式
        //  YES :   MOBILE 
        //  NO  :   硬件重启
        unsigned char *p;

        CLEAN_RX_BUFFER();
        Delay1ms(30);

        SEND_STRING(CMD_AIRPLANE);
        Delay1ms(100);  // 3s

        p = strstr(RX_BUFFER,"OK");
        if(p !=NULL)
        {
            STATUS = MOBILE;
        }        
    }

    else if(STATUS == MOBILE)
    {
        //  "AT+CFUN=1" ;       关闭飞行模式   
        //  YES :   NEWSMS 
        //  NO  :
        unsigned char *p;

        CLEAN_RX_BUFFER();
        Delay1ms(30);

        SEND_STRING(CMD_MOBILE);
        Delay1ms(30);   // 0.5s

        p = strstr(RX_BUFFER,"OK");   // "+CPIN: READY"
        if(p != NULL)
        {
            STATUS = REARY;
        }
    }

    else if(STATUS == REARY)
    {
        //  "AT" ;              尝试和模块通信
        //  YES :   ONLINE 
        //  NO  :   重启模块
        unsigned char *p;

        CLEAN_RX_BUFFER();
        Delay1ms(30);

        SEND_STRING(CMD_AT);
        Delay1ms(30);

        p = strstr(RX_BUFFER,"OK");
        if(p != NULL)
        {
            CLEAN_RX_BUFFER();
            STATUS = ONLINE;
        }
    }

    else if(STATUS == ONLINE)
    {
        //  "AT+CPIN?";         检查SIM状态
        //  YES :   SIM 
        //  NO  :   i > 20  AIRPLANE
        unsigned char *p;

        CLEAN_RX_BUFFER();
        Delay1ms(30);

        if(i >= 20)
        {
            STATUS = AIRPLANE;
            i=0;
        }
        else
        {
            SEND_STRING(CMD_SIM);
            Delay1ms(30);   // 1s

            p = strstr(RX_BUFFER,"+CPIN: READY");
            if(p != NULL)
            {
                STATUS = SIM;
                i=0;
            }
            else
            {
                i++;
            }
        }
    }

    else if(STATUS == SIM)
    {
        //  "AT+CREG"           检查是否注册到家庭网络(0,1)或漫游网络(0,5)
        //  YES :   REG 
        //  NO  :   i > 50  AIRPLANE 
        unsigned char *p;

        CLEAN_RX_BUFFER();
        Delay1ms(30);

        if(i > 50)
        {
            STATUS = AIRPLANE;
            i=0;
        }
        else
        { 
            SEND_STRING(CMD_REG);
            Delay1ms(30);   // 0.5s

            p = strstr(RX_BUFFER,"1");
            if(p != NULL)
            {
                STATUS = REG;
                i=0;
            }
            else
            {
                p = strstr(RX_BUFFER,"5");
                if(p != NULL)
                {
                    STATUS = REG;
                    i=0;
                }
                else
                {
                    i++;
                }
            }
        }
    }

    else if(STATUS == REG)
    {
        //  "AT+CSQ"            检查网络质量(0-31)
        //  YES :   SIGNAL 
        //  NO  :   i > 50  AIRPLANE
        unsigned char *p;

        CLEAN_RX_BUFFER();
        Delay1ms(30);

        if(i > 50)
        {
            STATUS = AIRPLANE;
            i=0;
        }
        else
        {
            SEND_STRING(CMD_SIGNAL);
            Delay1ms(30);

            // if(RX_BUFFER[6] > '0')
            // {

            //     CLEAN_RX_BUFFER();
            //     STATUS = SIGNAL;
            //     i=0;
            // }
            // else if(RX_BUFFER[7] > '0')
            // {

            //     CLEAN_RX_BUFFER();
            //     STATUS = SIGNAL;
            //     i=0;
            // }
            // else
            // {
            //     i++;
            //     CLEAN_RX_BUFFER();
            // }

            p = strstr(RX_BUFFER,"OK");
            if(p != NULL)
            {
                STATUS = SIGNAL;
            }
            else
            {
                i++;
            }
        }
        
        // p = strstr(RX_BUFFER,"OK");
        // if(p != NULL)
        // {
        //     STATUS = SIGNAL;
        //     CLEAN_RX_BUFFER();
        // }
    }

    else if(STATUS == SIGNAL)
    {
        //  "AT+CMPS?"           检查有没有新短信
        //  YES :   NEWSMS 
        //  NO  :   i > 20  READY
        unsigned char *p;
        unsigned char *g;
        char temp;
        int k;

        CLEAN_RX_BUFFER();
        Delay1ms(30);

        P0 = 0X80;

        if( i >= 50)
        {
            STATUS = REARY;
            i=0;
        }
        else
        {
            SEND_STRING(CMD_CKSMS);
            Delay1ms(30);

            g = strstr(RX_BUFFER,"SM_P");
            if(g != NULL)
            {
                g +=6;
                temp = *g;
                // g +=6;
                // SMS_CNT[0] = *g++;
                // SMS_CNT[1]  = *g;

                // if(SMS_CNT[1] == ',')
                // {
                //     SMS_CNT[1] = '\0';
                // }
                // if(*g >= '0' && *g <= '9');
                // {
                //     SMS_CNT[1]  =  *g;
                // }
            }
            // if(SMS_CNT[0] > '0')
            if(temp > '0')
            {
                for(k = 1; k <= 50; k++)
                {
                    CLEAN_TX_BUFFER();
                    Delay1ms(30);

                    CHANGE_BUFFER = 1;
                    // TX_INDEX = 0;

                    SEND_STRING(CMD_GTSMS);
                    if(k >= 10)
                    {
                        SEND_BYTE(TOCHAR(k/10));
                        SEND_BYTE(TOCHAR(k%10));
                    }else
                    {
                        SEND_BYTE(TOCHAR(k));
                    }
                    SEND_BYTE(0X0D);
                    SEND_BYTE(0X0A);

                    Delay1ms(50);
                    CHANGE_BUFFER = 0;

                    if(strlen(TX_BUFFER) >= 36)
                    {
                        p = strstr(TX_BUFFER,"+CM");
                        if(p != NULL)
                        {
                            if(k >= 10)
                            {
                                SMS_CNT[0] = TOCHAR(k/10);
                                SMS_CNT[1] = TOCHAR(k%10);
                            }else
                            {
                                SMS_CNT[0] = TOCHAR(k);
                                SMS_CNT[1] = '\0';
                            }			
                            STATUS = NEWSMS;
                            TX_INDEX = 0;
                            i = 0;
                            FORMAT_TX_BUFFER();
                            Delay1ms(30);

                            break;
                        }
                    }
                    // DEBUG();
                }
            }
            else
            {
                i++;
            }
        }
        P0 = 0X00;
    }

    else if(STATUS == SHUT)
    {
        //  "AT+CIPSHUT"             关闭移动场景
        //  YES :   SIGNAL 
        //  NO  :   AIRPLANE
        unsigned char *p;
        
        // SEND_STRING(CMD_SHUTGPRS);
        // Delay1ms(5);
        CLEAN_RX_BUFFER();
        Delay1ms(30);

        SEND_STRING(CMD_SHUT);
        Delay1ms(30);

        p = strstr(RX_BUFFER,"SHUT OK");
        if(p != NULL)
        {
            STATUS = SIGNAL;
        }
        else
        {
            STATUS = AIRPLANE;
        }      
    }

    else if(STATUS == NEWSMS)
    {
        //  "AT+CSTT=\"CMNET\""         设置CMNET
        //  YES :   APN 
        //  NO  :   SHUT
        unsigned char *p;

        CLEAN_RX_BUFFER();
        Delay1ms(30);

        SEND_STRING(CMD_APN);
        Delay1ms(50);

        p = strstr(RX_BUFFER,"OK");
        if(p != NULL)
        {
            STATUS = APN;
        }
        else
        {
            STATUS = SHUT;
        }       
    }

    else if(STATUS == APN)
    {
        //  "AT+CIICR"                  开启GPRS
        //  YES :   GPRS 
        //  NO  :   SHUT
        unsigned char *p;

        CLEAN_RX_BUFFER();
        Delay1ms(30);

        SEND_STRING(CMD_GPRS);
        Delay1ms(50);

        p = strstr(RX_BUFFER,"OK");
        if(p != NULL)
        {
            STATUS = GPRS;
        }
        else
        {
            STATUS = SHUT;
        }        
    }

    else if(STATUS == GPRS)
    {
        //  "AT+CIFSR"                  查询获取的IP
        //  YES :   IP 
        //  NO  :   SHUT
        unsigned char *p;

        CLEAN_RX_BUFFER();
        Delay1ms(30);

        SEND_STRING(CMD_IP);
        Delay1ms(30);

        p = strstr(RX_BUFFER,".");
        if(p != NULL)
        {
            STATUS = IP;
        }
    }

    else if(STATUS == IP)
    {
        //  "AT+CIPSTART=\"TCP\",\"8.8.8.8\",\"443\""       连接远程服务器 
        //  YES :   SERVER 
        //  NO  :   SHUT
        unsigned char *p;

        CLEAN_RX_BUFFER();
        Delay1ms(30);

        SEND_STRING(CMD_SERVER);
        Delay1ms(50);

        p = strstr(RX_BUFFER,"CONNECT OK");
        if(p != NULL)
        {
            STATUS = SERVER;
        }
        else
        {
            STATUS = SHUT;
        }
    }

    else if(STATUS == SERVER)
    {
        //  "AT+CIPSEND"                发送短信 
        //  YES :   SIGNAL 
        //  NO  :   SERVER  i > 2 SHUT
        unsigned char *p;
        unsigned char *g;
        int j;
        int k;

        CLEAN_RX_BUFFER();
        Delay1ms(30);

        if(i > 2)
        {
            STATUS = SHUT;
            i=0;
        }
        else
        {
            SEND_STRING(CMD_SEND);
            Delay1ms(30);

            p = strstr(RX_BUFFER,">");
            if(p != NULL)
            {
                // CLEAN_RX_BUFFER();
                // Delay1ms(30);

                for(j = 0;j < strlen(TX_BUFFER); j++)
                {
                    SEND_BYTE(TX_BUFFER[j]);
                }
                SEND_BYTE(0X1A);
                Delay1ms(30); //延迟太低就收不到反馈

                g = strstr(RX_BUFFER,"OK");  //  "SEND OK"
                if(g != NULL)
                {

                    SEND_STRING(CMD_DLSMS);
                    

                    for(k = 0; k < strlen(SMS_CNT); k++)
                    {
                        SEND_BYTE(SMS_CNT[k]);
                    }

                    SEND_BYTE(0X0D);
                    SEND_BYTE(0X0A);

                    CLEAN_TX_BUFFER();
                    STATUS = SHUT;
                    i=0;

                }
                else
                {
                    i++;
                }
            }
            else
            {
                i++;
            } 
            DEBUG();           
        }        
    }
}

//  1.发送 AT 和模块通信
//  2.发送 AT+CPIN? 检测SIM卡状态
//  3.发送 AT+CREG? 查询入网情况
//  3.发送 AT+CSQ 查询信号质量
//  4.发送 AT+CGATT? 查询GPRS附着情况


//  5.发送 AT+CIPSHUT 关闭移动场景
//  6.发送 AT+CSTT="CMNET" 设置APN
//  7.发送 AT+CIICR 激活移动场景
//  8.发送 AT+CIFSR 查询IP地址
//  9.发送 AT+CIPSTART="TCP","193.112.94.54","9001" 连接服务器
// 10.发送 AT+CIPSEND 收到'>'后，发送数据
// 11.发送 AT+CIPCLOSE 关闭连接
// 12.发送 AT+CIPSHUT 关闭移动场景


void SEND_BYTE(unsigned char c)
{
    while(!TI);
    TI = 0;
    SBUF = c;
}


void SEND_STRING(unsigned char *p)
{
    while(*p != '\0')
    {
        SEND_BYTE(*p++);
    }
}

void CLEAN_RX_BUFFER()
{
    memset(RX_BUFFER,0,sizeof(RX_BUFFER));
    RX_INDEX = 0;
}

void CLEAN_TX_BUFFER()
{
    memset(TX_BUFFER,0,sizeof(TX_BUFFER));
    TX_INDEX = 0;
}

void FORMAT_TX_BUFFER()
{
    char *ps;
    int i;

    ps = strstr(TX_BUFFER,",\"\",");
    if(ps != NULL)
    {
        ps += 7;
        if(*ps == 0X0A)
        {
            ps += 1;
            for(i = 0; *ps != '\0'; i++)
            {
                if(*ps == 0X20)
                {
                    TX_BUFFER[i] = '\0';
                    break;
                }else if (*ps == 0X0D)
                 {
                    TX_BUFFER[i] = '\0';
                    break;
                }else if (*ps == 0X0A)
                 {
                    TX_BUFFER[i] = '\0';
                    break;
                }else
                {
                    TX_BUFFER[i] = *ps++;
                }
            }

        }else if (*ps == 0X0D) 
        {
            ps += 2;
            for(i = 0; *ps != '\0'; i++)
            {
                if(*ps == 0X20)
                {
                    TX_BUFFER[i] = '\0';
                    break;
                }else if (*ps == 0X0D)
                 {
                    TX_BUFFER[i] = '\0';
                    break;
                }else if (*ps == 0X0A)
                 {
                    TX_BUFFER[i] = '\0';
                    break;
                }else
                {
                    TX_BUFFER[i] = *ps++;
                }
            }
        }        
    }
}

//  50  感觉是1s
//  100 感觉是3s
//  200 感觉是7s
//  500 感觉是17s
void Delay1ms(unsigned int i)
{
 	unsigned int j;
	while(i--)
	{
	 	for(j = 0; j < 125; j++);
	}
}

void SERIAL() interrupt 4
{
    if(RI)
    {
        if(!CHANGE_BUFFER)
        {
            RI = 0;
            RX_BUFFER[RX_INDEX++] = SBUF;

            if(RX_INDEX >= RX_LEN - 1)
            {
                RX_INDEX = 0;
            }
        }
        else
        {
            RI = 0;
            TX_BUFFER[TX_INDEX++] = SBUF;

            if(TX_INDEX >= TX_LEN - 1)
            {
                TX_INDEX = 0;
            }
        }
        
    }
}

void UART()
{
    SCON = 0X52;
    PCON = 0X80;
    TMOD = 0X20;
    TH1  = 0XFD;
    TR1  = 0XFD;

    ES   = 1;
    EA   = 1;
    TR1  = 1;
}

void DEBUG()
{
    Delay1ms(100);

    SEND_BYTE('D');
    SEND_BYTE('E');
    SEND_BYTE('B');
    SEND_BYTE('U');
    SEND_BYTE('G');
    SEND_BYTE(':');

    SEND_STRING(RX_BUFFER);

    SEND_BYTE(':');
    SEND_BYTE('D');
    SEND_BYTE('E');
    SEND_BYTE('B');
    SEND_BYTE('U');
    SEND_BYTE('G');

    Delay1ms(100);
}