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
#include "KeccakF-1600-reference.h"

#define nrRounds 24
uint32 KeccakRoundConstants[nrRounds][2];
#define nrLanes 25
uint32 KeccakRhoOffsets[nrLanes];

#define index(x, y,z) ((((x)%5)+5*((y)%5))*2 + z)

void _rol(uint32 *result,uint32 *a,uint32 length)
{
	uint32 i;
	uint32 len_div,len_mod;

	len_div=(length>>5) % 2;
	len_mod=length & 0x1f;
	for (i=0;i<2;i++)
	{
		if (!len_mod)
			result[(i+2+len_div) % 2]=a[i];
		else
			result[(i+2+len_div) % 2]=(a[i]<<len_mod) | (a[(i+2-1) % 2]>>(32-len_mod));
	}
}

void theta(uint32 *A)
{
    uint32 x, y, z;
    uint32 C[5][2], D[5][2];

    for (x=0; x<5; x++)
	{
        for (z=0; z<2; z++) 
		{
            C[x][z] = 0; 
            for (y=0; y<5; y++)
                C[x][z] ^= A[index(x, y, z)];
        }
    }
    for (x=0; x<5; x++) 
	{
		_rol(D[x],C[(x+1)%5],1);
        for (z=0; z<2; z++)
            D[x][z] ^= C[(x+4)%5][z];
    }
    for (x=0; x<5; x++)
        for (y=0; y<5; y++)
            for (z=0; z<2; z++)
                A[index(x, y, z)] ^= D[x][z];
}

void rho(uint32 *A)
{
    uint32 x, y;
	uint32 temp[2];

    for (x=0; x<5; x++) 
		for (y=0; y<5; y++)
		{
			_rol(temp,&A[(x+5*y)*2],KeccakRhoOffsets[x+5*y]);
			A[(x+5*y)*2]=temp[0];
			A[(x+5*y)*2+1]=temp[1];
		}
}

void pi(uint32 *A)
{
    uint32 x, y, z;
    uint32 tempA[nrLanes*2];

    for (x=0; x<5; x++) 
		for (y=0; y<5; y++)
			for (z=0; z<2; z++)
				tempA[index(x, y, z)] = A[index(x, y, z)];
    for (x=0; x<5; x++)
		for (y=0; y<5; y++)
			for(z=0; z<2; z++)
				A[index(0*x+1*y, 2*x+3*y, z)] = tempA[index(x, y, z)];
}

void chi(uint32 *A)
{
    uint32 x, y, z;
    uint32 C[5][2];

    for (y=0; y<5; y++)
	{ 
        for (x=0; x<5; x++)
            for (z=0; z<2; z++)
                C[x][z] = A[index(x, y, z)] ^ ((~A[index(x+1, y, z)]) & A[index(x+2, y, z)]);
        for (x=0; x<5; x++)
            for (z=0; z<2; z++)
                A[index(x, y, z)] = C[x][z];
    }
}

void iota(uint32 *A, uint32 indexRound)
{
    A[index(0, 0, 0)] ^= KeccakRoundConstants[indexRound][0];
    A[index(0, 0, 1)] ^= KeccakRoundConstants[indexRound][1];
}

int LFSR86540(uint8 *LFSR)
{
    int result = ((*LFSR) & 0x01) != 0;
    
	if (((*LFSR) & 0x80) != 0)// Primitive polynomial over GF(2): x^8+x^6+x^5+x^4+1
        (*LFSR) = ((*LFSR) << 1) ^ 0x71;
    else
        (*LFSR) <<= 1;

    return result;
}

void KeccakInitializeRoundConstants()
{
    uint8 LFSRstate = 0x01;
    uint32 i, j, bitPosition;
	uint32 temp[2]={1,0};
	uint32 result[2];

    for (i=0; i<nrRounds; i++) 
	{
        KeccakRoundConstants[i][0]=KeccakRoundConstants[i][1] = 0;
        for (j=0; j<7; j++) 
		{
            bitPosition = (1<<j)-1; //2^j-1
            if (LFSR86540(&LFSRstate))
			{
				_rol(result,temp,bitPosition);
				KeccakRoundConstants[i][0]^=result[0];
				KeccakRoundConstants[i][1]^=result[1];
			}
        }
    }
}

void KeccakInitializeRhoOffsets(void)
{
    uint32 x, y, t, newX, newY;

    KeccakRhoOffsets[0] = 0;
    x = 1;
    y = 0;
    for(t=0; t<24; t++)
	{
        KeccakRhoOffsets[5*y+x] = ((t+1)*(t+2)/2) % 64;
        newX = (0*x+1*y) % 5;
        newY = (2*x+3*y) % 5;
        x = newX;
        y = newY;
    }
}

void KeccakInitialize()
{
    KeccakInitializeRoundConstants();//initial round const(KeccakRoundConstants)
    KeccakInitializeRhoOffsets();//initial rho offset(KeccakRhoOffsets)
}

void KeccakInitializeState(uint8 *state)
{
    memset(state, 0, KeccakPermutationSizeInBytes);
}

#ifdef ProvideFast1024
void KeccakExtract1024bits(const uint8 *state, uint8 *data)
{
    memcpy(data, state, 128);
}
#endif

void KeccakExtract(const uint8 *state, uint8 *data, uint32 laneCount)
{
    memcpy(data, state, laneCount*8);
}

void KeccakPermutationOnWords(uint32 *state)
{
    uint32 i;

    for(i=0;i<nrRounds;i++) 
	{
        theta(state);
        rho(state);
        pi(state);
        chi(state);
        iota(state,i);
    }
}

void KeccakPermutation(uint8 *state)
{
//#if (PLATFORM_BYTE_ORDER != IS_LITTLE_ENDIAN)
//    uint64 stateAsWords[KeccakPermutationSize/64];
//#endif

//#if (PLATFORM_BYTE_ORDER == IS_LITTLE_ENDIAN)
	//n round theta/rho/pi/chi/iota
    KeccakPermutationOnWords((uint32 *)state);
//#else
//    fromBytesToWords(stateAsWords, state);
//    KeccakPermutationOnWords(stateAsWords);
//    fromWordsToBytes(state, stateAsWords);
//#endif
}

void KeccakPermutationAfterXor(uint8 *state, const uint8 *data, uint32 dataLengthInBytes)
{
    uint32 i;

    for(i=0; i<dataLengthInBytes; i++)
        state[i] ^= data[i];
    KeccakPermutation(state);
}

#ifdef ProvideFast576
void KeccakAbsorb576bits(uint8 *state, const uint8 *data)
{
    KeccakPermutationAfterXor(state, data, 72);
}
#endif

#ifdef ProvideFast832
void KeccakAbsorb832bits(uint8 *state, const uint8 *data)
{
    KeccakPermutationAfterXor(state, data, 104);
}
#endif

#ifdef ProvideFast1024
void KeccakAbsorb1024bits(uint8 *state, const uint8 *data)
{
    KeccakPermutationAfterXor(state, data, 128);
}
#endif

#ifdef ProvideFast1088
void KeccakAbsorb1088bits(uint8 *state, const uint8 *data)
{
    KeccakPermutationAfterXor(state, data, 136);
}
#endif

#ifdef ProvideFast1152
void KeccakAbsorb1152bits(uint8 *state, const uint8 *data)
{
    KeccakPermutationAfterXor(state, data, 144);
}
#endif

#ifdef ProvideFast1344
void KeccakAbsorb1344bits(uint8 *state, const uint8 *data)
{
    KeccakPermutationAfterXor(state, data, 168);
}
#endif

void KeccakAbsorb(uint8 *state, const uint8 *data, uint32 laneCount)
{
    KeccakPermutationAfterXor(state, data, laneCount*8);
}
