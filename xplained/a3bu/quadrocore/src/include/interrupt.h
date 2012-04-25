#ifndef INTERRUPT_H_
#define INTERRUPT_H_


void PMICInit(uint8_t level);
void EnableGlobalInterrupts(void);
void DisableGlobalInterrupts(void);

#endif /* INTERRUPT_H_ */