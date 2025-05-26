/*
 * memory.h
 *
 *  Created on: May 25, 2025
 *      Author: willw
 */

#ifndef INC_MEMORY_H_
#define INC_MEMORY_H_

void* arrAlloc(uint8_t size, size_t elemSize);
void** matrixAlloc(uint8_t matrixSize, uint8_t arrSize, size_t elemSize);
void* arrRealloc(void* arr, uint8_t currentSize, uint8_t increase, size_t elemSize);
void** matrixRealloc(void** matrix, uint8_t currentMatrixSize, uint8_t matrixIncrease, uint8_t arrSize, size_t elemSize);
void freeMatrix(void** matrix, uint8_t matrixSize);



#endif /* INC_MEMORY_H_ */
