#pragma once //sha3 algorithm

//include
#include "include.h"
#include "KeccakF-1600-reference.h"
//define
//struct
typedef struct
{
    uint8 buffer[KeccakPermutationSize/8];
    uint8 dataQueue[KeccakMaximumRate/8];
    uint32 rate;
    uint32 capacity;
    uint32 bitsInQueue;
    uint32 fixedOutputLength;
    uint32 bitsAvailableForSqueezing;
    uint8 squeezing;//0-not squeez,1-squeez
}sha3_ctx;
//function
void sha3_init(uint32 bitlen);
void sha3_update(uint8 *content,uint32 length);
void sha3_final(uint8 *result);
//private
/**
  * Function to initialize the state of the Keccak[r, c] sponge function.
  * The sponge function is set to the absorbing phase.
  * @param  state       Pointer to the state of the sponge function to be initialized.
  * @param  rate        The value of the rate r.
  * @param  capacity    The value of the capacity c.
  * @pre    One must have r+c=1600 and the rate a multiple of 64 bits in this implementation.
  * @return Zero if successful, 1 otherwise.
  */
int InitSponge(sha3_ctx *ctx, uint32 rate, uint32 capacity);
/**
  * Function to give input data for the sponge function to absorb.
  * @param  state       Pointer to the state of the sponge function initialized by InitSponge().
  * @param  data        Pointer to the input data. 
  *                     When @a databitLen is not a multiple of 8, the last bits of data must be
  *                     in the least significant bits of the last byte.
  * @param  databitLen  The number of input bits provided in the input data.
  * @pre    In the previous call to Absorb(), databitLen was a multiple of 8.
  * @pre    The sponge function must be in the absorbing phase,
  *         i.e., Squeeze() must not have been called before.
  * @return Zero if successful, 1 otherwise.
  */
int Absorb(sha3_ctx *ctx, const uint8 *data, uint32 databitlen);
/**
  * Function to squeeze output data from the sponge function.
  * If the sponge function was in the absorbing phase, this function 
  * switches it to the squeezing phase.
  * @param  state       Pointer to the state of the sponge function initialized by InitSponge().
  * @param  output      Pointer to the buffer where to store the output data.
  * @param  outputLength    The number of output bits desired.
  *                     It must be a multiple of 8.
  * @return Zero if successful, 1 otherwise.
  */
int Squeeze(sha3_ctx *ctx, uint8 *output, uint32 outputLength);
