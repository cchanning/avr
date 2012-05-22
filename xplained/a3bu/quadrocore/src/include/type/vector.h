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

/**
 *	These macros should be used when directly storing pointers in a vector, they will ensure
 *	that the pointer dereferencing occurs properly. For example:
 *	
 *	uint8_t value = 1;
 *	uint8_t *valueP = &value;
 *	Vector_t *vectorP = VectorAlloc(1, sizeof(ptr_t));
 *	VectorAddRow(vectorP, &valueP);
 *	uint8_t *rowP = VectorGetDeferencedRow(vectorP, 0, uint8_t*)
 *		
 */
#define VectorGetDeferencedRow(vectorP, index, type) ( ((index > -1) && (index < vectorP->rowCount) ? (type)*((uint16_t*)(vectorP->rowP + (index * vectorP->rowSize))) : NULL) )

/**
 *	These macros should be used when storing blocks of memory in a vector. 	
 */
#define VectorGetRow(vectorP, index, type) ( ((index > -1) && (index < vectorP->rowCount) ? (type)(vectorP->rowP + (index * vectorP->rowSize)) : NULL) )

typedef struct _Vector
{
	uint8_t incUnit;
	uint16_t maxRows;
	uint16_t rowCount;
	uint16_t rowSize;
	ptr_t rowP;
} Vector_t;

/**
 * This function will allocate memory for a new vector.
 * 
 * @param incUnit the number of rows to add each time to the vector is to be grown
 * @param rowSize the size of row to be stored in the vector memory block
 * @return a pointer to the vector
 */
Vector_t* VectorAlloc(uint8_t incUnit, uint16_t rowSize);

/**
 * Creates a row within the vector. Note that the pointer returned should not be stored as it could
 * be invalidated if rows are removed, throw it away once your target structure etc has been initialised.
 *
 * @param vectorP a pointer to the vector
 * @return a temporary pointer to the block of memory (rowSize)
 */
ptr_t VectorCreateRow(Vector_t *vectorP);

/**
 * Adds a row within the vector memory block and copy the dereferenced contents of the
 * row. Pointers may be stored directly within the vector provided the address of the pointer that is required 
 * to be stored is passed in, this will ensure that the address of the "real" pointer is copied correctly. 
 
 *  For example:
 *
 *	uint8_t value = 1;
 *	uint8_t *valueP = &value;
 *  Vector_t *vectorP = VectorAlloc(1, sizeof(ptr_t));
 *  ptr_t p1 = VectorAddRow(vectorP, &vP);
 *
 * @param vectorP a pointer to the vector
 * @param rowP a pointer to the row to be copied
 * @return if the add was successful
 */
bool_t VectorAddRow(Vector_t *vectorP, ptr_t rowP);

/**
 * Removes the specified row from the vector, note that the removal may cause the
 * vector elements to be realigned
 *
 * @param vectorP a pointer to the vector
 * @param index a row index
 * @return if the the row was removed successfully
 */
bool_t VectorRemoveRow(Vector_t *vectorP, uint16_t rowIndex);

/**
 * Expands the vector by vectorP->incUnit.
 *
 * @param vectorP a pointer to the vector
 * @return if the vector was expanded
 */
bool_t VectorGrow(Vector_t *vectorP);

/**
 * Returns the size of the vector (row count).
 *
 * @param vectorP a pointer to the vector
 * @return the size of the vector (row count)
 */
uint16_t VectorSize(Vector_t *vectorP);

/**
 * Clears the vector.
 *
 * @param vectorP a pointer to the vector
 */
void VectorClear(Vector_t *vectorP);

/**
 * Frees the vector
 *
 * @param vectorP a pointer to the vector
 */
void VectorFree(Vector_t *vectorP);

#endif /* VECTOR_H_ */