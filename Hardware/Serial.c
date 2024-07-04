#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>
#include "Serial.h"
#include "Delay.h"

#include "jm_client.h"
#include "debug.h"

/*
uint8_t Serial_RxData;
uint8_t Serial_RxFlag;

*/

/*
static Serial_onBufData uart1Cb;
static Serial_onBufData uart2Cb;
*/

#define RECV_TIMEOUT 1000 //接收数据1秒超时

#define MAX_SIZE 1024

static uint8_t bufIdx = 1;

static uint8_t rxBuffer[MAX_SIZE];
static uint16_t rxBufferLen = 0;

static uint8_t rxBuffer0[MAX_SIZE];
static uint16_t rxBufferLen0 = 0;

int getRXBuffer(uint8_t *pBuffer, int bufferLen, uint8_t bidx) {
	
	uint8_t *rxbuf = bidx ? rxBuffer : rxBuffer0;
	uint16_t rxBufLen = bidx ? rxBufferLen : rxBufferLen0;
	
  if (rxBufferLen > bufferLen) {
    memcpy(pBuffer,rxbuf, bufferLen);
    memcpy(rxBuffer, rxbuf + rxBufLen, rxBufLen-bufferLen);
    //rxBufferLen = ;
	bufIdx?(rxBufferLen =rxBufLen - bufferLen):(rxBufferLen0 = rxBufLen - bufferLen); 
    return bufferLen;
  }
  
  int sizeReturned = rxBufLen;
  memcpy(pBuffer, rxbuf, rxBufLen);
  //rxBufferLen = 0;
  bidx ? (rxBufferLen = 0) : (rxBufferLen0 = 0);
  return sizeReturned;
}

static Serial_onByteData uart1ByteCb;
static Serial_onByteData uart2ByteCb;

static int8_t uart1ProType = JM_UART_PRO_NOMAL;
static int8_t uart2ProType = JM_UART_PRO_NETPCK;

//USART_TypeDef* uartNo
void Serial_Init(uint8_t uartNo, Serial_onByteData dcb, int8_t proType)
{
	if(uartNo == JM_UART1){
		
		if(dcb && proType == JM_UART_PRO_NOMAL) {
			uart1ByteCb =  (Serial_onByteData)dcb;
		}/* else {
			uart1Cb = (Serial_onBufData)dcb;
		}
		*/
		uart1ProType = proType;
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		USART_InitTypeDef USART_InitStructure;
		USART_InitStructure.USART_BaudRate = 115200;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_Init(USART1, &USART_InitStructure);
		
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
		
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
		
		NVIC_InitTypeDef NVIC_InitStructure;
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_Init(&NVIC_InitStructure);
		
		USART_Cmd(USART1, ENABLE);
	
	}else if(uartNo == JM_UART2){

		if(proType == JM_UART_PRO_NOMAL) {
			uart2ByteCb =  (Serial_onByteData)dcb;
		}/*else {
			uart2Cb = (Serial_onBufData)dcb;
		}
		*/
		
		uart2ProType = proType;
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		USART_InitTypeDef USART_InitStructure;
		USART_InitStructure.USART_BaudRate = 38400; //115200;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_Init(USART2, &USART_InitStructure);
		
		USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
		
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
		
		NVIC_InitTypeDef NVIC_InitStructure;
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_Init(&NVIC_InitStructure);
		
		USART_Cmd(USART2, ENABLE);
	}
}

void Serial_SendByte(uint8_t jmUartNo, uint8_t Byte)
{
	USART_TypeDef* uno = jmUartNo == JM_UART1 ? USART1: USART2;
	USART_SendData(uno, Byte);
	while (USART_GetFlagStatus(uno, USART_FLAG_TXE) == RESET);
}

void Serial_SendArray(uint8_t jmUartNo, uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)
	{
		Serial_SendByte(jmUartNo, Array[i]);
	}
}

void Serial_SendString(uint8_t jmUartNo, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)
	{
		Serial_SendByte(jmUartNo, String[i]);
	}
}

uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y --)
	{
		Result *= X;
	}
	return Result;
}

void Serial_SendNumber(uint8_t jmUartNo, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)
	{
		Serial_SendByte(jmUartNo, Number / Serial_Pow(10, Length - i - 1) % 10 + '0');
	}
}

int fputc(int ch, FILE *f)
{
	uint8_t uartNo = uart1ProType == JM_UART_PRO_NOMAL ? JM_UART1: JM_UART2;
	Serial_SendByte(uartNo,ch);
	return ch;
}

void Serial_Printf(uint8_t jmUartNo, char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	Serial_SendString(jmUartNo, String);
}

void Serial_SendJmPckArray(uint8_t jmUartNo, uint8_t *Array, uint16_t len)
{
	Serial_SendByte(jmUartNo, (len>>8) & 0xFF );//长度高字节
	Serial_SendByte(jmUartNo, len & 0xFF );//长度低字节
	Serial_SendArray(jmUartNo,Array,len);
}

void Serial_SendJmPckBuf(uint8_t jmUartNo, jm_buf_t *buf)
{
	uint16_t len = jm_buf_readable_len(buf);
	
	Serial_SendByte(jmUartNo, (len>>8) & 0xFF );//长度高字节
	Serial_SendByte(jmUartNo, len & 0xFF );//长度低字节
	
	uint8_t v=0;
	for(int i =0; i < len; i++) {
		jm_buf_get_u8(buf,&v);
		Serial_SendByte(jmUartNo,v);
	}
}

//写包长度
void Serial_writeLen(uint8_t jmUartNo, uint16_t len){
	Serial_SendByte(jmUartNo, (len>>8) & 0xFF );//长度高字节
	Serial_SendByte(jmUartNo, len & 0xFF );//长度低字节
}

//写入Bug，不包括长度
void Serial_writeBuf(uint8_t jmUartNo, jm_buf_t *buf){
	uint16_t len = jm_buf_readable_len(buf);	
	uint8_t v=0;
	for(int i =0; i < len; i++) {
		jm_buf_get_u8(buf,&v);
		Serial_SendByte(jmUartNo,v);
		if(i%100==0) {
			Delay_ms(2);
		}
	}
}

void USART1_IRQHandler(void)
{
	static USART_TypeDef* uartNo =  USART1;
	//static uint8_t uartNo =  JM_UART1;

	uint8_t byte = USART_ReceiveData(USART1);
	if(uart1ProType == JM_UART_PRO_NOMAL) {
		if(uart1ByteCb) uart1ByteCb(JM_UART1, byte);
	} else {
		static jm_buf_t *buf = NULL;
		static uint16_t dataSize = 0;
		static uint8_t hs = 0;
		
		if(hs < 2) {
			if(hs == 0) {
				hs++;
				dataSize = byte<<8;
				USART_ClearITPendingBit(uartNo, USART_IT_RXNE);
				return;
			}else if(hs == 1) {
				hs++;//gotSize最大值是2，表示后面都是数据，gotSize不再增加
				dataSize = dataSize | byte;
				//前两个字节是包的长度，最大65535个字节
				if(dataSize > 0) {//不能发空包
					buf = jm_buf_create(dataSize);
				}
				//JINFO("One uartNo=%d len=%d\n",JM_UART1, byte);
				USART_ClearITPendingBit(uartNo, USART_IT_RXNE);
				return;
			}
		}
		
		jm_buf_put_u8(buf,byte);
		if(jm_buf_is_full(buf)) {
			//Serial_Printf("Finish uartNo=%d\n",JM_UART1);
			jm_cli_getJmm()->jm_postEvent(JM_TASK_APP_RX_DATA, JM_UART1, buf, JM_EVENT_FLAG_DEFAULT);
			buf = NULL;
			dataSize = 0;
			hs = 0;
		}
	}
	USART_ClearITPendingBit(uartNo, USART_IT_RXNE);
}

void USART2_IRQHandler(void)
{
	
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != SET){
		return;
	}
	
	//static USART_TypeDef* uartNo = USART2;
	uint8_t byte = USART_ReceiveData(USART2);
	
	if(uart2ProType == JM_UART_PRO_NOMAL) {
		if(uart2ByteCb) uart2ByteCb(JM_UART2, byte);
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	} else {
		
		static uint16_t dataSize = 0;
		static uint16_t recvSize = 0;
		static uint8_t ds = 0;
		static uint64_t lastRecvTime = 0;
		
		if(lastRecvTime && (jm_cli_getSysTime() - lastRecvTime) > RECV_TIMEOUT) {
			dataSize = 0;
			recvSize = 0;
			ds = 0;
			
			 if(bufIdx) {
				if(rxBufferLen) {
					//SINFO("buf1 tclr\n");
				}
				rxBufferLen = 0;
			} else {
				if(rxBufferLen0) {
					//SINFO("buf0 tclr\n");
				}
				rxBufferLen0 = 0;
			}
		 
		}
		
		lastRecvTime = jm_cli_getSysTime();
		//SINFO("recvBuf=%u dataSize=%d hs=%d\n",byte, dataSize, hs);
	
		if(ds < 2) {
			if(ds == 0) {
				ds=1;
				dataSize = byte << 8;
				USART_ClearITPendingBit(USART2, USART_IT_RXNE);
				return;
			}else if(ds == 1) {	
				dataSize |= byte;
				//前两个字节是包的长度，最大65535个字节
				if(dataSize > MAX_SIZE || dataSize == 0) {
					SINFO("tlong ds=%u max=%d\n",dataSize,MAX_SIZE);
					dataSize = 0;
					ds = 0;
					//SINFO("ds=%d\n",dataSize);
					USART_ClearITPendingBit(USART2, USART_IT_RXNE);
					return;
				}
				
				ds = 2;//gotSize最大值是2，表示后面都是数据，gotSize不再增加
				//SINFO("m\n");
				//SINFO("ds=%d\n",dataSize);
				USART_ClearITPendingBit(USART2, USART_IT_RXNE);
				return;
			}
		}

		  if(bufIdx) {
			   if(rxBufferLen < MAX_SIZE) {
					rxBuffer[rxBufferLen] = byte;
					rxBufferLen++;
			   }
		  } else {
			   if(rxBufferLen0 < MAX_SIZE) {
					rxBuffer0[rxBufferLen0] = byte;
					rxBufferLen0++;
			   }
		  }
		 
		recvSize++;
		//SINFO("%d ", idx);
		
		if(recvSize == dataSize) {
			//SINFO("Finish uartNo=%d ds=%d\n",JM_UART2,dataSize);
			//Serial_canRecv(false);
			#if 0
			if(bufIdx) {
				JM_UDP_DEBUG("rxBuffer p\n");
				for(int i=0; i < dataSize; i++) {
					os_printf("%u ",rxBuffer[i]);
				}
				os_printf("\n");
			} else {
				JM_UDP_DEBUG("rxBuffer0 p\n");
				for(int i=0; i < dataSize; i++) {
					os_printf("%u ",rxBuffer0[i]);
				}
				os_printf("\n");
			}
			
			#endif //UDP_DEBUG_ENABLE

			jm_cli_getJmm()->jm_postEvent(JM_TASK_APP_RX_DATA, dataSize, (void*)((uint32)bufIdx), 0);
			
			bufIdx = bufIdx==0?1:0;
			if(bufIdx) {
				if(rxBufferLen) {
					//SINFO("buf1 clr\n");
				}
				rxBufferLen = 0;
				//memset(rxBuffer,0,MAX_SIZE);
			} else {
				if(rxBufferLen0) {
					//SINFO("buf0 clr\n");
				}
				rxBufferLen0 = 0;
				//memset(rxBuffer0,0,MAX_SIZE);
			}
			  
			dataSize = 0;
			recvSize = 0;
			ds = 0;
			//SINFO("p end\n");
		}
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
	
}


