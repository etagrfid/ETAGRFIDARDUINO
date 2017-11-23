/*
 * ManchesterUtil.cpp
 *
 * Created: 11/14/2017 7:25:36 PM
 *  Author: Jay
 */ 
#include <Arduino.h>

#include "ManchesterUtil.h"

int has_even_parity(uint16_t x,int datasize)
{
	volatile unsigned int count = 0, i, b = 1;

	for(volatile int i = 0; i < datasize; i++)
	{
		if( x & (b << i) )
		{
			count++;
		}
	}

	if( (count % 2)==0 )
	{
		return 0;
	}
	
	return 1;
}

int CheckManchesterParity(EM4100Data *xd)
{
	int row_err_count=0;
	int col_err_count=0;
	int err_count = 0;
	for(int i=0;i<10;i++)
	{
		bool this_row = !has_even_parity(xd->lines[i].data_nibb,4);
		if(this_row != xd->lines[i].parity)
		row_err_count++;
	}
	for(int i=0;i<4;i++)
	{
		volatile uint16_t coldata = 0x00;
		uint16_t imask = 0x01 << i;
		for(int ii=0;ii<10;ii++)
		{
			uint16_t thisbit = xd->lines[ii].data_nibb;
			thisbit &= imask;
			thisbit >>= i;
			coldata |= thisbit  << ii;
		}
		
		bool this_colp = !has_even_parity(coldata,10);
		volatile uint8_t readparity = xd->colparity;
		uint8_t read_col_par_bit = (readparity & imask) >> i;
		if(this_colp != read_col_par_bit)
		col_err_count++;
	}
	err_count = col_err_count+row_err_count;
}