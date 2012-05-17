#ifndef VECTOR_H_
#define VECTOR_H_

#define VectorGetRowByIndex(vectorP, index, type) ( (type)*((uint16_t*)(vectorP->rowP + (index * vectorP->rowSize))) )
#define VectorGetRow(vectorP, rowP, type) ( (type)*((uint16_t*)(rowP)) )

typedef struct _Vector
{
	uint8_t incUnit;
	uint16_t maxRows;
	uint16_t rowCount;
	uint16_t rowSize;
	ptr_t rowP;
} Vector_t;

Vector_t* VectorAlloc(uint8_t incUnit, uint16_t rowSize);
ptr_t VectorAddRow(Vector_t *vectorP, ptr_t rowP);
bool VectorRemoveRow(Vector_t *vectorP, ptr_t rowP);
bool VectorGrow(Vector_t *vectorP);

#endif /* VECTOR_H_ */