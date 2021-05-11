#include <string.h>
#include "eth_que.h"


typedef struct
{
    uint8_t in;
    uint8_t out;
    uint8_t count;
    uint8_t *Data[QUE_BUFFER_SIZE];
    uint16_t Size[QUE_BUFFER_SIZE];
} ethFIFO_TypeDef; /* 송신 패킷 저장 구조체 */

ethFIFO_TypeDef rxQue;

/**
 * @brief Put the Eth Que Object
 * 
 * @param rxData: 버퍼에 저장할 데이터 포인터
 * @param size: 데이터 크기(byte)
 * @return int: 버퍼에 데이터가 다 찾으면 FALSE
 */
int putEthQue(uint8_t *rxData, uint16_t size)
{
    int status = FALSE;
    if (rxQue.count != QUE_BUFFER_SIZE) /* 데이터가 버퍼에 가득 차지 않았으면 */
    {
        rxQue.Data[rxQue.in] = rxData;
        rxQue.Size[rxQue.in] = size;
        rxQue.in++;
        rxQue.count++;                   /* 버퍼에 저장된 갯수 1 증가 */
        if (rxQue.in == QUE_BUFFER_SIZE) /* 시작 인덱스가 버퍼의 끝이면 */
        {
            rxQue.in = 0U; /* 시작 인덱스를 0부터 다시 시작 */
        }
        status = TRUE;
    }
    else
    {
        status = FALSE;
    }
    return status;
}

/**
 * @brief Get the Eth Que object
 * 
 * @param rxData: 버퍼에서 반환될 데이터 포인터
 * @return int: 데이터 크기(byte). 0이면 버퍼에 데이터가 없음
 */
int getEthQue(uint8_t *rxData)
{
    int dataSize = 0;
    if (rxQue.count != 0U) /* 버퍼에 데이터가 있으면 */
    {
    	dataSize = (int)rxQue.Size[rxQue.out];
        memcpy(rxData, rxQue.Data[rxQue.out], dataSize);
        rxQue.Data[rxQue.out] = (void *)0;
        rxQue.out++;
        rxQue.count--;                    /* 버퍼에 저장된 데이터 갯수 1 감소 */
        if (rxQue.out == QUE_BUFFER_SIZE) /* 끝 인덱스가 버퍼의 끝이면 */
        {
            rxQue.out = 0U; /* 끝 인덱스를 0부터 다시 시작 */
        }
    }

    return dataSize;
}
