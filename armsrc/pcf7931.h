#ifndef __PCF7931_H
#define __PCF7931_H

size_t DemodPCF7931(uint8_t **outBlocks);
uint8_t IsBlock0PCF7931(uint8_t *Block);
uint8_t IsBlock1PCF7931(uint8_t *Block);
void ReadPCF7931();
void SendCmdPCF7931(uint32_t * tab);
bool AddBytePCF7931(uint8_t byte, uint32_t * tab, int32_t l, int32_t p);
bool AddBitPCF7931(bool b, uint32_t * tab, int32_t l, int32_t p);
bool AddPatternPCF7931(uint32_t a, uint32_t b, uint32_t c, uint32_t * tab);
void WritePCF7931(uint8_t pass1, uint8_t pass2, uint8_t pass3, uint8_t pass4, uint8_t pass5, uint8_t pass6, uint8_t pass7, uint16_t init_delay, int32_t l, int32_t p, uint8_t address, uint8_t byte, uint8_t data);
void BruteForcePCF7931(uint64_t start_password, uint8_t tries, uint16_t init_delay, int32_t l, int32_t p);

#endif
