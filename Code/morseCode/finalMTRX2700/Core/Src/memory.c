#include "main.h"

// Dynamically allocate memory for an array
void* arrAlloc(uint8_t size, size_t elemSize) {
    void* arr = calloc(size, elemSize);
    if (arr == NULL) {
        exit(1);  // Allocation failed
    }
    return arr;
}

// Allocate a 2D matrix (matrixSize rows, each with arrSize elements)
void** matrixAlloc(uint8_t matrixSize, uint8_t arrSize, size_t elemSize) {
    void** arr = calloc(matrixSize, sizeof(void*));
    if (arr == NULL) {
        exit(1);
    }

    for (uint8_t i = 0; i < matrixSize; i++) {
        arr[i] = arrAlloc(arrSize, elemSize);
    }
    return arr;
}

// Reallocate a 1D array with extra capacity
void* arrRealloc(void* arr, uint8_t currentSize, uint8_t increase, size_t elemSize) {
    void* newArr = realloc(arr, (currentSize + increase) * elemSize);
    if (newArr == NULL) {
        exit(1);
    }
    return newArr;
}

// Reallocate matrix with more rows
void** matrixRealloc(void** matrix, uint8_t currentMatrixSize, uint8_t matrixIncrease, uint8_t arrSize, size_t elemSize) {
    void** newArr = realloc(matrix, (currentMatrixSize + matrixIncrease) * sizeof(void*));
    if (newArr == NULL) {
        exit(1);
    }

    for (uint8_t i = currentMatrixSize; i < (currentMatrixSize + matrixIncrease); i++) {
        newArr[i] = arrAlloc(arrSize, elemSize);
    }
    return newArr;
}

// Free all rows, then the outer matrix
void freeMatrix(void** matrix, uint8_t matrixSize) {
    if (!matrix) return;
    for (uint8_t i = 0; i < matrixSize; i++) {
        free(matrix[i]);
    }
    free(matrix);  // Also free the outer array
}
