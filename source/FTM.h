#ifndef FTM_H_
#define FTM_H_

#include <stdint.h>
#define FTM_CHANNEL     1
#define FTM_MOD_COUNT   62      // Periodo ~1.25us
#define FTM_VAL_0       20      // ~0.4us (0 lógico)
#define FTM_VAL_1       40      // ~0.8us (1 lógico)
#define FTM_VAL_RESET   1       // <--- TRUCO: 20ns. El LED lo ve como 0, el FTM lo ve como disparo.

void FTM_Init(void); 
#endif /* FTM_H_ */