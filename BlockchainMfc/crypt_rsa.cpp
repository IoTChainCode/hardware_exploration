#include "stdafx.h"
#include "crypt_rsa.h"

//uint8 *content:initial random
//uint32 length:prime byte length(key length/2)
void rsa_genprime(uint8 *content,uint32 length)
{
	//gen prime(small prime number trial division)
	uint32 i,j,k,r;
	uint8 prime[2],*quotient,*remainder;
	const uint8 table_smallprime[96]=
	{
		0x03,0x05,0x07,0x0b,0x0d,0x11,0x13,
		0x17,0x1d,0x1f,0x25,0x29,0x2b,0x2f,
        0x35,0x3b,0x3d,0x43,0x47,0x49,0x4f,
        0x53,0x59,0x61,0x65,0x67,0x6b,0x6d,
        0x71,0x7f,0x83,0x89,0x8b,0x95,0x97,
        0x9d,0xa3,0xa7,0xad,0xb3,0xb5,0xbf,
        0xc1,0xc5,0xc7,0xd3,0xdf,0xe3,0xe5,
        0xe9,0xef,0xf1,0xfb,0x01,0x07,0x0D,
        0x0F,0x15,0x19,0x1B,0x25,0x33,0x37,
        0x39,0x3D,0x4B,0x51,0x5B,0x5D,0x61,
        0x67,0x6F,0x75,0x7B,0x7F,0x85,0x8D,
        0x91,0x99,0xA3,0xA5,0xAF,0xB1,0xB7,
        0xBB,0xC1,0xC9,0xCD,0xCF,0xD3,0xDF,
        0xE7,0xEB,0xF3,0xF7,0xFD
	};

	content[length-1] |= 0xc0;//msb 2 bits=11
	content[0] |= 0x01;//lsb bit=1
	quotient=new uint8[length-1+1];
	remainder=new uint8[2];
    while(1)
    {
    	j=0;
        for (i=0;i<96;i++)
        {
            prime[0]=table_smallprime[i];
			prime[1]=i>52 ? 0x01 : 0x00;
			_div(quotient,remainder,content,prime,length,2,k,r);
			if (!_cmp(remainder,0,r))//remainder=0
            {
                j=1;
                break;
            }
        }
        if (!j && !rsa_verifyprime(content,length))
            break;
        _add(content,content,2,length);//step +2
    }
	delete[] quotient;
	delete[] remainder;
}

//uint8 *content:prime
//uint32 length:prime byte
//uint8:0-pass,1-fail
uint8 rsa_verifyprime(uint8 *content,uint32 length)
{
	//miller-rabin algorithm verify prime(partially detectable for composite number)
    uint8 i;
	uint32 j,k;
	uint8 *temp,*value;
 
	//shift right content-1
	temp=new uint8[length];
	_sub(temp,&i,content,1,length);
	j=0;
    while(!(temp[0] & 0x01))//loop if lsb=0
    {
    	_sr(temp,temp,length,1);//shift right m
    	j++;
    }
	if (!j)//content-1's lsb=1(composite number)
	{
		delete[] temp;
		return 1;
	}
	//value=2^temp mod content
	value=new uint8[length*2];
	_modexp(value,2,temp,content,length,length);
	if (!_cmp(value,1,length))
	{
		delete[] temp;
		delete[] value;
		return 0;
	}
	k=1;
	while(1)
	{
		_add(temp,value,1,length);
		if (!_cmp(temp,content,length,length))//exit if value+1=content
			break;
		if (j==k)//composite number detected
		{
			delete[] temp;
			delete[] value;
			return 1;
		}
        //value=value*value mod content
		_modmul(value,value,value,content,length,length,length);
        k++;
	}
	delete[] temp;
	delete[] value;

    return 0;
}

//rsa_t *rsa:rsa para structure
//rsa->e:rsa key_e
//uint32 config:0-std,1-crt
//uint8:0-pass,1-fail
uint8 rsa_genkey(rsa_t *rsa,uint32 config)
{
	uint8 i;
	uint32 len,length;
	uint8 *m;

	len=rsa->len>>1;
    //gen prime p/q
    rsa_genprime(rsa->p,len);
    rsa_genprime(rsa->q,len);
	//test 1:if p==q then fail
	if (!_cmp(rsa->p,rsa->q,len,len))
    	return 1;
    //test 2:if e/m not coprime then fail
	m=new uint8[rsa->len];
    _mul(rsa->n,rsa->p,rsa->q,len,len);//n=p*q
	_sub(rsa->p,&i,rsa->p,1,len);//p=p-1
	_sub(rsa->q,&i,rsa->q,1,len);//q=q-1
    _mul(m,rsa->p,rsa->q,len,len);//euler function:m=(p-1)*(q-1)
    i=_modinv(rsa->d,rsa->e,m,rsa->le,rsa->len,length);//key_d d=e^-1%m
	if (i)//e/m not coprime
	{
		delete[] m;
		return 1;
	}
    if (config)
    {
    	//crt dec:dp=d%(p-1),dq=d%(q-1),qp=(q^-1)%p
	    _div(m,rsa->dp,rsa->d,rsa->p,rsa->len,len,length,length);
	    _div(m,rsa->dq,rsa->d,rsa->q,rsa->len,len,length,length);
    	//restore p/q
		_add(rsa->p,rsa->p,1,len);
		_add(rsa->q,rsa->q,1,len);
		_modinv(rsa->qp,rsa->q,rsa->p,len,len,length);
    }
    else
    {
    	//restore p/q
		_add(rsa->p,rsa->p,1,len);
		_add(rsa->q,rsa->q,1,len);
    }
	delete[] m;

    return 0;
}

//uint8 *result:output ciphertext
//uint8 *content:input plaintext
//uint32 length:text byte
//rsa_t *rsa:rsa para structure
//rsa->e:rsa key_e
//rsa->n:eular
uint32 rsa_enc(uint8 *result,uint8 *content,uint32 length,rsa_t *rsa)
{
	//C=M^e%n
	return _modexp(result,content,rsa->e,rsa->n,length,rsa->le,rsa->len);
}

//uint8 *result:output plaintext
//uint8 *content:input ciphertext
//uint32 length:text byte
//rsa_t *rsa:rsa para structure
//rsa->p:
//rsa->q:
//rsa->d:(use when config=0)
//rsa->n:(use when config=0)
//rsa->dp:(use when config=1)
//rsa->dq:(use when config=1)
//rsa->qp:(use when config=1)
//uint32 config:0-std,1-crt
uint32 rsa_dec(uint8 *result,uint8 *content,uint32 length,rsa_t *rsa,uint32 config)
{
	uint8 i;
	uint8 *m1,*m2,*temp,*rand;
	uint32 len,length_1,length_2;
	uint32 len_d;
	
	if (rsa->lr)
	{
		rand=new uint8[rsa->lr];
		_rand(rand,rsa->lr);
	}
	len=rsa->len>>1;
	if (!config)
	{
		if (rsa->lr)//exponentially blind(std),rsa->d should malloc rand size
		{
			temp=new uint8[len+len+rsa->lr];
			m1=new uint8[len];
			m2=new uint8[len];
			//m=(p-1)*(q-1)
			_sub(m1,&i,rsa->p,1,len);
			_sub(m2,&i,rsa->q,1,len);
			len_d=_mul(temp,m1,m2,len,len);
			//d'=d+rand*m
			len_d=_mul(temp,temp,rand,len_d,rsa->lr);
			len_d=_add(rsa->d,rsa->d,temp,rsa->len,len_d);
			//release
			delete[] m1;
			delete[] m2;
			delete[] temp;
		}
		else
			len_d=rsa->len;
		//m=c^d mod n
		length=_modexp(result,content,rsa->d,rsa->n,length,len_d,rsa->len);
	}
	else
	{
		temp=new uint8[len];
		m1=new uint8[length+len];
		m2=new uint8[length+len];
		//c1=c mod p,m1=(c1 ^ dp) mod p
		length_1=_mod(temp,content,rsa->p,length,len);
		length_1=_modexp(m1,temp,rsa->dp,rsa->p,length_1,len,len);
		//c2=c mod q,m2=(c2 ^ dq) mod q
		length_2=_mod(temp,content,rsa->q,length,len);
		length_2=_modexp(m2,temp,rsa->dq,rsa->q,length_2,len,len);
		//m=m2+[(m1-m2)*qp mod p]*q
		length=_modsub(result,m1,m2,rsa->p,length_1,length_2,len);
		length=_modmul(result,result,rsa->qp,rsa->p,length,len,len);
		length=_mul(result,result,rsa->q,length,len);
		length=_add(result,result,m2,length,length_2);
		//release
		delete[] m1;
		delete[] m2;
		delete[] temp;
	}
	if (rsa->lr)
		delete[] rand;

    return length;
}