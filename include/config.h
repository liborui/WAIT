#ifndef CONFIG_H
#define CONFIG_H
#define d_m3CodePageFreeLinesThreshold      4+2       // max is: select _sss & CallIndirect + 2 for bridge

#define d_m3MemPageSize                     65535 //65535

#define d_m3Reg0SlotAlias                   30000
#define d_m3Fp0SlotAlias                    30001

#define d_m3MaxSaneUtf8Length               2000
#define d_m3MaxSaneFunctionArgCount         1000    // still insane, but whatever
#endif