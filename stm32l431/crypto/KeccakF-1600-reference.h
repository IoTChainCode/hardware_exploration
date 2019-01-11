/*
The Keccak sponge function, designed by Guido Bertoni, Joan Daemen,
Michal Peeters and Gilles Van Assche. For more information, feedback or
questions, please refer to our website: http://keccak.noekeon.org/

Implementation by the designers,
hereby denoted as "the implementer".

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/
*/
#ifndef _KeccakF_Reference_h_
#define _KeccakF_Reference_h_

//include
#include "include.h"
//define
#define ProvideFast576
#define ProvideFast832
#define ProvideFast1024
#define ProvideFast1088
#define ProvideFast1152
#define ProvideFast1344

#define KeccakPermutationSize 1600
#define KeccakPermutationSizeInBytes (KeccakPermutationSize/8)
#define KeccakMaximumRate 1536
#define KeccakMaximumRateInBytes (KeccakMaximumRate/8)
//function
void KeccakInitialize(void);
void KeccakInitializeState(uint8 *state);
void KeccakPermutation(uint8 *state);
#ifdef ProvideFast576
void KeccakAbsorb576bits(uint8 *state, const uint8 *data);
#endif
#ifdef ProvideFast832
void KeccakAbsorb832bits(uint8 *state, const uint8 *data);
#endif
#ifdef ProvideFast1024
void KeccakAbsorb1024bits(uint8 *state, const uint8 *data);
#endif
#ifdef ProvideFast1088
void KeccakAbsorb1088bits(uint8 *state, const uint8 *data);
#endif
#ifdef ProvideFast1152
void KeccakAbsorb1152bits(uint8 *state, const uint8 *data);
#endif
#ifdef ProvideFast1344
void KeccakAbsorb1344bits(uint8 *state, const uint8 *data);
#endif
void KeccakAbsorb(uint8 *state, const uint8 *data, uint32 laneCount);
#ifdef ProvideFast1024
void KeccakExtract1024bits(const uint8 *state, uint8 *data);
#endif
void KeccakExtract(const uint8 *state, uint8 *data, uint32 laneCount);

#endif
