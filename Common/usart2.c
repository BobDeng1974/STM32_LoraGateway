/**
  ******************************************************************************
  * @file    STM32F407DEMO/usart2.c 
  * @author  Liang
  * @version V1.0.0
  * @date    2018-1-22
  * @brief   
  ******************************************************************************
  * @attention
	* �����rt-thread��֧��
	*	����rt-thread msh�Ĵ�������
  ******************************************************************************
**/


/* include-------------------------------------------------- */
#include "sys.h"
#include "usart2.h"
#include "delay.h"
#include "rtthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
//#include "utils.h"

volatile unsigned char  gprs_ready_flag = 0;
volatile unsigned char  gprs_ready_count = 0;

char  usart2_rcv_buf[MAX_RCV_LEN];
volatile unsigned int   usart2_rcv_len = 0;

//Ӳ�������ڳ�ʼ��
void uart2_init(u32 bound)
{
   //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//ʹ��USART2ʱ��
 
	//����2��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); //GPIOA2����ΪUSART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); //GPIOA3����ΪUSART2
	
	//USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PA2��PA3

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure); //��ʼ������2
	
	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =4;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

	USART_Cmd(USART2, ENABLE);  //ʹ�ܴ���2
	
	USART_ClearFlag(USART2, USART_FLAG_TC);
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//��������ж�
	
}

/**
 * ��մ���2���ջ���
 * @param   
 * @return 
 * @brief 
 **/
void USART2_Clear(void)
{
	memset(usart2_rcv_buf, 0, strlen(usart2_rcv_buf));
	usart2_rcv_len = 0;
}

/**
 * ���ڷ���
 * @param   
 * @return 
 * @brief 
 **/
void USART2_Write(USART_TypeDef* USARTx, uint8_t *Data, uint8_t len)
{
    uint8_t i;
    USART_ClearFlag(USARTx, USART_FLAG_TC);
    for(i = 0; i < len; i++)
    {
        USART_SendData(USARTx, *Data++);
        while( USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET );
    }
}

/**
 * ���ڷ���ATָ��
 * @param   
 * @return 
 * @brief 
 **/
void SendCmd(char* cmd, char* result, int timeOut)
{
    while(1)
    {
        USART2_Clear();
        USART2_Write(USART2, cmd, strlen(cmd));
        delay_ms(timeOut);
        if((NULL != strstr(usart2_rcv_buf, result)))	//�ж��Ƿ���Ԥ�ڵĽ��
        {
            break;
        }
        else
        {
            delay_ms(100);
        }
    }
}

/**
 * ���ش��ڽ��ܵ������ݳ���
 * @param   
 * @return 
 * @brief 
 **/
uint32_t USART2_GetRcvNum(void)
{
	static uint32_t len = 0;
	uint32_t result = 0;
	
	if(usart2_rcv_len == 0)
	{
		len = 0;
		result = 0;
	}
	else if(len != usart2_rcv_len)
	{
		result = usart2_rcv_len - len;	//�½��ճ���
		len = usart2_rcv_len;			//�����³���
	}
	
	return result;
}

/**
 * ���ƴ��ڽ��յ������ݵ�buf
 * @param   
 * @return 
 * @brief 
 **/
void  USART2_GetRcvData(uint8_t *buf, uint32_t rcv_len)
{
    if(buf)
    {
        memcpy(buf, usart2_rcv_buf, rcv_len);
    }
    //USART2_Clear();
}
