#include "quadrocore.h"

Vector_t* VectorAlloc(uint8_t incUnit, uint16_t rowSize)
{
	Vector_t *vectorP = NULL;
	
	if ((incUnit <= 0) || (rowSize <= 0))
	{
		return NULL;
	}
	
	if (! (vectorP = calloc(1, sizeof(Vector_t))))
	{
		return NULL;
	}
	
	vectorP->incUnit = incUnit;
	vectorP->rowSize = rowSize;
	vectorP->maxRows = 0;
	vectorP->rowCount = 0;
	vectorP->rowP = NULL;
	
	return vectorP;
}

ptr_t VectorAddRow(Vector_t *vectorP, ptr_t rowP)
{
	ptr_t newRowP = NULL;
	
	if (! vectorP)
	{
		return NULL;
	}
	
	if (vectorP->rowCount == vectorP->maxRows)
	{
		if (! VectorGrow(vectorP))
		{
			return NULL;
		}
	}
	
	newRowP = (ptr_t)((uint8_t *)vectorP->rowP) + (vectorP->rowCount * vectorP->rowSize);
	
	if (! memcpy(newRowP, rowP, vectorP->rowSize))
	{
		return NULL;
	}
	
	vectorP->rowCount++;		
	
	return newRowP;
}

bool_t VectorRemoveRow(Vector_t *vectorP, ptr_t rowP)
{
	return true;
}

bool_t VectorGrow(Vector_t *vectorP)
{
	if (! vectorP)
	{
		return false;
	}
	
	vectorP->maxRows += vectorP->incUnit;
	
	if (! (vectorP->rowP = realloc(vectorP->rowP, vectorP->maxRows * vectorP->rowSize)))
	{
		return false;
	}
	
	return true;
}
