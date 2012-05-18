/***********************************************************************************************************************
 * 
 * > QuadroCore <
 * 
 * Copyright (C) 2012 by Chris Channing
 *
 ***********************************************************************************************************************
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 ***********************************************************************************************************************/

#ifndef VECTOR_H_
#define VECTOR_H_

/*
	These macros should be used when directly storing pointers in a vector, they will ensure
	that the pointer dereferencing occurs properly. For example:
	
	uint8_t value = 1;
	uint8_t *valueP = &value;
	Vector_t *vectorP = VectorAlloc(1, sizeof(ptr_t));
	ptr_t p1 = VectorAddRow(vectorP, &vP);
	uint8_t *rowP = VectorGetRowPtr(vectorP, p1, uint8_t*)
		
 */
#define VectorGetDeferencedRowByIndex(vectorP, index, type) ( (type)*((uint16_t*)(vectorP->rowP + (index * vectorP->rowSize))) )
#define VectorGetDeferencedRow(vectorP, rowP, type) ( (type)*((uint16_t*)(rowP)) )

/*
	These macros should be used when storing blocks of memory in a vector. For example:
	
	Vector_t *vectorP = VectorAlloc(1, sizeof(AStruct_t));
	ptr_t p1 = VectorCreateRow(vectorP);
	AStruct_t *aStructP = VectorGetRow(vectorP, p1, AStruct_t*);
		
 */
#define VectorGetRowByIndex(vectorP, index, type) ( (type)(vectorP->rowP + (index * vectorP->rowSize) )
#define VectorGetRow(vectorP, rowP, type) ( (type)rowP )

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
ptr_t VectorCreateRow(Vector_t *vectorP);
bool_t VectorRemoveRow(Vector_t *vectorP, ptr_t rowP);
bool_t VectorGrow(Vector_t *vectorP);

#endif /* VECTOR_H_ */