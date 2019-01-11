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
#include "crypt_sha3.h"

sha3_ctx ctx3;

//public
void sha3_init(uint32 bitlen)
{
	switch(bitlen)
	{
    case 0://arbitrary length output
        InitSponge(&ctx3,1024,576);
        break;
    case 224:
        InitSponge(&ctx3,1152,448);
        break;
    case 256:
        InitSponge(&ctx3,1088,512);
        break;
    case 384:
        InitSponge(&ctx3,832,768);
        break;
    case 512:
        InitSponge(&ctx3,576,1024);
        break;
	default:
		break;
    }
    ctx3.fixedOutputLength=bitlen;
}

//content:input data
//length:bit length
void sha3_update(uint8 *content,uint32 length)
{
	uint8 last;

    if (!(length%8))
        Absorb(&ctx3,content,length);
    else 
	{
        Absorb(&ctx3,content,length-length%8);
		//Align the last partial byte to the least significant bits
		last=content[length/8]>>(8-length%8);
		Absorb(&ctx3,&last,length%8);
    }
}

void sha3_final(uint8 *result)
{
	Squeeze(&ctx3,result,ctx3.fixedOutputLength);
}

//private
int InitSponge(sha3_ctx *ctx, uint32 rate, uint32 capacity)
{
    if (rate+capacity != 1600)
        return 1;
    if ((rate <= 0) || (rate >= 1600) || ((rate % 64) != 0))
        return 1;
    KeccakInitialize();
    ctx->rate = rate;
    ctx->capacity = capacity;
    ctx->fixedOutputLength = 0;
    KeccakInitializeState(ctx->buffer);
    memset(ctx->dataQueue, 0, KeccakMaximumRateInBytes);
    ctx->bitsInQueue = 0;
    ctx->squeezing = 0;
    ctx->bitsAvailableForSqueezing = 0;

    return 0;
}

void AbsorbQueue(sha3_ctx *ctx)
{
    // ctx->bitsInQueue is assumed to be equal to ctx->rate
#ifdef ProvideFast576
    if (ctx->rate == 576)
        KeccakAbsorb576bits(ctx->buffer, ctx->dataQueue);
    else 
#endif
#ifdef ProvideFast832
    if (ctx->rate == 832)
        KeccakAbsorb832bits(ctx->buffer, ctx->dataQueue);
    else 
#endif
#ifdef ProvideFast1024
    if (ctx->rate == 1024)
        KeccakAbsorb1024bits(ctx->buffer, ctx->dataQueue);
    else 
#endif
#ifdef ProvideFast1088
    if (ctx->rate == 1088)
        KeccakAbsorb1088bits(ctx->buffer, ctx->dataQueue);
    else
#endif
#ifdef ProvideFast1152
    if (ctx->rate == 1152)
        KeccakAbsorb1152bits(ctx->buffer, ctx->dataQueue);
    else 
#endif
#ifdef ProvideFast1344
    if (ctx->rate == 1344)
        KeccakAbsorb1344bits(ctx->buffer, ctx->dataQueue);
    else 
#endif
    KeccakAbsorb(ctx->buffer, ctx->dataQueue, ctx->rate/64);
    ctx->bitsInQueue = 0;
}

int Absorb(sha3_ctx *ctx, const uint8 *data, uint32 databitlen)
{
    uint32 i, j, wholeBlocks;
    uint32 partialBlock, partialByte;
    const uint8 *curData;

    if ((ctx->bitsInQueue % 8) != 0)
        return 1; // Only the last call may contain a partial byte
    if (ctx->squeezing)
        return 1; // Too late for additional input
    i = 0;
    while(i < databitlen) 
	{
        if ((ctx->bitsInQueue == 0) && (databitlen >= ctx->rate) && (i <= (databitlen-ctx->rate))) 
		{
            wholeBlocks = (databitlen-i)/ctx->rate;
            curData = data+i/8;
#ifdef ProvideFast576
            if (ctx->rate == 576) 
			{
                for(j=0; j<wholeBlocks; j++, curData+=576/8) 
				{
                    KeccakAbsorb576bits(ctx->buffer, curData);
                }
            }
            else
#endif
#ifdef ProvideFast832
            if (ctx->rate == 832) 
			{
                for(j=0; j<wholeBlocks; j++, curData+=832/8) 
				{
                    KeccakAbsorb832bits(ctx->buffer, curData);
                }
            }
            else
#endif
#ifdef ProvideFast1024
            if (ctx->rate == 1024) 
			{
                for(j=0; j<wholeBlocks; j++, curData+=1024/8) 
				{
                    KeccakAbsorb1024bits(ctx->buffer, curData);
                }
            }
            else
#endif
#ifdef ProvideFast1088
            if (ctx->rate == 1088) 
			{
                for(j=0; j<wholeBlocks; j++, curData+=1088/8) 
				{
                    KeccakAbsorb1088bits(ctx->buffer, curData);
                }
            }
            else
#endif
#ifdef ProvideFast1152
            if (ctx->rate == 1152) 
			{
                for(j=0; j<wholeBlocks; j++, curData+=1152/8) 
				{
                    KeccakAbsorb1152bits(ctx->buffer, curData);
                }
            }
            else
#endif
#ifdef ProvideFast1344
            if (ctx->rate == 1344) 
			{
                for(j=0; j<wholeBlocks; j++, curData+=1344/8) 
				{
                    KeccakAbsorb1344bits(ctx->buffer, curData);
                }
            }
            else
#endif
            {
                for(j=0; j<wholeBlocks; j++, curData+=ctx->rate/8) 
				{
                    KeccakAbsorb(ctx->buffer, curData, ctx->rate/64);
                }
            }
            i += wholeBlocks*ctx->rate;
        }
        else 
		{
            partialBlock = (uint32)(databitlen - i);
            if (partialBlock+ctx->bitsInQueue > ctx->rate)
                partialBlock = ctx->rate-ctx->bitsInQueue;
            partialByte = partialBlock % 8;
            partialBlock -= partialByte;
            memcpy(ctx->dataQueue+ctx->bitsInQueue/8, data+i/8, partialBlock/8);
            ctx->bitsInQueue += partialBlock;
            i += partialBlock;
            if (ctx->bitsInQueue == ctx->rate)
                AbsorbQueue(ctx);
            if (partialByte > 0) 
			{
                uint8 mask = (1 << partialByte)-1;
                ctx->dataQueue[ctx->bitsInQueue/8] = data[i/8] & mask;
                ctx->bitsInQueue += partialByte;
                i += partialByte;
            }
        }
    }

    return 0;
}

void PadAndSwitchToSqueezingPhase(sha3_ctx *ctx)
{
    // Note: the bits are numbered from 0=LSB to 7=MSB
    if (ctx->bitsInQueue + 1 == ctx->rate) 
	{
        ctx->dataQueue[ctx->bitsInQueue/8 ] |= 1 << (ctx->bitsInQueue % 8);
        AbsorbQueue(ctx);
        memset(ctx->dataQueue, 0, ctx->rate/8);
    }
    else 
	{
        memset(ctx->dataQueue + (ctx->bitsInQueue+7)/8, 0, ctx->rate/8 - (ctx->bitsInQueue+7)/8);
        ctx->dataQueue[ctx->bitsInQueue/8 ] |= 1 << (ctx->bitsInQueue % 8);
    }
    ctx->dataQueue[(ctx->rate-1)/8] |= 1 << ((ctx->rate-1) % 8);
    AbsorbQueue(ctx);
#ifdef ProvideFast1024
    if (ctx->rate == 1024) 
	{
        KeccakExtract1024bits(ctx->buffer, ctx->dataQueue);
        ctx->bitsAvailableForSqueezing = 1024;
    }
    else
#endif
    {
        KeccakExtract(ctx->buffer, ctx->dataQueue, ctx->rate/64);
        ctx->bitsAvailableForSqueezing = ctx->rate;
    }
    ctx->squeezing = 1;
}

int Squeeze(sha3_ctx *ctx, uint8 *output, uint32 outputLength)
{
    uint32 i;
    uint32 partialBlock;

    if (!ctx->squeezing)
        PadAndSwitchToSqueezingPhase(ctx);
    if ((outputLength % 8) != 0)
        return 1; // Only multiple of 8 bits are allowed, truncation can be done at user level
    i = 0;
    while(i < outputLength) 
	{
        if (ctx->bitsAvailableForSqueezing == 0) 
		{
            KeccakPermutation(ctx->buffer);
#ifdef ProvideFast1024
            if (ctx->rate == 1024) 
			{
                KeccakExtract1024bits(ctx->buffer, ctx->dataQueue);
                ctx->bitsAvailableForSqueezing = 1024;
            }
            else
#endif
            {
                KeccakExtract(ctx->buffer, ctx->dataQueue, ctx->rate/64);
                ctx->bitsAvailableForSqueezing = ctx->rate;
            }
        }
        partialBlock = ctx->bitsAvailableForSqueezing;
        if (partialBlock > outputLength - i)
            partialBlock = outputLength - i;
        memcpy(output+i/8, ctx->dataQueue+(ctx->rate-ctx->bitsAvailableForSqueezing)/8, partialBlock/8);
        ctx->bitsAvailableForSqueezing -= partialBlock;
        i += partialBlock;
    }

    return 0;
}
