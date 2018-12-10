#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <fstream>

#include "utils.h"

// CRC32
uint32_t CRC_encode(const char* data, int len)
{
#define CRC_poly_32 0x04c11db7

    int byte_count=0, bit_count=0;
    uint32_t CRC = 0xffffffff;

    while (byte_count < len)
    {
        if (((uint8_t)(CRC>>31)^(*data>>(7-bit_count++)&1)) != 0)
            CRC = CRC << 1^CRC_poly_32;
        else 
            CRC = CRC << 1;

        if (bit_count > 7)
        {
            bit_count = 0;
            byte_count++;
            data++;
        }
    }

    return CRC;
}

/* remark:接口函数定义 */  
int bits_initwrite(BITS_BUFFER_S *p_buffer, int i_size, unsigned char *p_data)  
{  
    if (!p_data)  
    {  
        return -1;  
    }  
    p_buffer->i_size = i_size;  
    p_buffer->i_data = 0;  
    p_buffer->i_mask = 0x80;  
    p_buffer->p_data = p_data;  
    p_buffer->p_data[0] = 0;  
    return 0;  
}  
  
void bits_align(BITS_BUFFER_S *p_buffer)  
{  
    if (p_buffer->i_mask != 0x80 && p_buffer->i_data < p_buffer->i_size)  
    {  
        p_buffer->i_mask = 0x80;  
        p_buffer->i_data++;  
        p_buffer->p_data[p_buffer->i_data] = 0x00;  
    }  
}  
  
void bits_write(BITS_BUFFER_S *p_buffer, int i_count, unsigned long i_bits)  
{  
    while (i_count > 0)  
    {  
        i_count--;  
  
        if ((i_bits >> i_count ) & 0x01)  
        {  
            p_buffer->p_data[p_buffer->i_data] |= p_buffer->i_mask;  
        }  
        else  
        {  
            p_buffer->p_data[p_buffer->i_data] &= ~p_buffer->i_mask;  
        }  
        p_buffer->i_mask >>= 1;  
        if (p_buffer->i_mask == 0)  
        {  
            p_buffer->i_data++;  
            p_buffer->i_mask = 0x80;  
        }  
    }  
}  
    
int bits_initread(BITS_BUFFER_S *p_buffer, int i_size, unsigned char *p_data)  
{  
    if (!p_data)  
    {  
        return -1;  
    }  
    p_buffer->i_size = i_size;  
    p_buffer->i_data = 0;  
    p_buffer->i_mask = 0x80;  
    p_buffer->p_data = p_data;  
    return 0;  
}  
  
int bits_read(BITS_BUFFER_S *p_buffer, int i_count, unsigned long *i_bits)  
{  
    if (!i_bits)  
    {  
        return -1;  
    }  
    *i_bits = 0;  
      
    while (i_count > 0)  
    {  
        i_count--;  
  
        if (p_buffer->p_data[p_buffer->i_data] & p_buffer->i_mask)  
        {  
            *i_bits |= 0x01;  
        }  
  
        if (i_count)  
        {  
            *i_bits = *i_bits << 1;  
        }  
          
        p_buffer->i_mask >>= 1;  
        if(p_buffer->i_mask == 0)  
        {  
            p_buffer->i_data++;  
            p_buffer->i_mask = 0x80;  
        }  
    }  
  
    return 0;  
}
