#ifndef ETH_QUE_H__
#define ETH_QUE_H__ 1

#include "platform.h"

#define QUE_BUFFER_SIZE 100 /*!< 이더넷 버퍼 갯수 */

int putEthQue(uint8_t *rxData, uint16_t size);
int getEthQue(uint8_t *rxData);

#endif /* ETH_QUE_H__ */
