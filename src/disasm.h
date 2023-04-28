#ifndef _DISASM_H
#define _DISASM_H 1

#include "config.h"
#include "hp48.h"

#define HP_MNEMONICS 0
#define CLASS_MNEMONICS 1

extern int disassembler_mode;
extern const char* mode_name[];

extern word_20 disassemble( word_20 addr, char* out );

#endif /* !_DISASM_H */
