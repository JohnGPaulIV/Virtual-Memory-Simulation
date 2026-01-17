/*
 * CS 261 PA2: Mini-ELF loader
 *
 * Name: John Gilbert Paul IV
 */

#include "p2-load.h"
#define FLAG_X 0x1
#define FLAG_W 0x2
#define FLAG_R 0x4

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

bool read_phdr (FILE *file, uint16_t offset, elf_phdr_t *phdr)
{
    if(!(file && phdr))
    {
        return false;
    }
    unsigned char byte[20];
    if(fseek(file, offset, SEEK_SET) != 0)
    {
        return false;
    }
    if(fread(byte, 1, sizeof(byte), file) < sizeof(byte)){
        return false;
    }

    phdr->p_offset = (uint32_t)byte[0] | ((uint32_t)byte[1] << 8) | ((uint32_t)byte[2] << 16) | ((uint32_t)byte[3] << 24);
    phdr->p_size = (uint32_t)byte[4] | ((uint32_t)byte[5] << 8) | ((uint32_t)byte[6] << 16) | ((uint32_t)byte[7] << 24);
    phdr->p_vaddr = (uint32_t)byte[8] | ((uint32_t)byte[9] << 8) | ((uint32_t)byte[10] << 16) | ((uint32_t)byte[11] << 24);
    phdr->p_type = (uint32_t)byte[12] | ((uint32_t)byte[13] << 8);
    phdr->p_flags = (uint32_t)byte[14] | ((uint32_t)byte[15] << 8);
    phdr->magic = (uint32_t)byte[16] | ((uint32_t)byte[17] << 8) | ((uint32_t)byte[18] << 16) | ((uint32_t)byte[19] << 24);
    
    if(!(phdr->magic == 0xDEADBEEF))
    {
        return false;
    }

    return true;
}

bool load_segment (FILE *file, byte_t *memory, elf_phdr_t *phdr)
{
    if(!(file && phdr && memory))
    {
        return false;
    }
    if(fseek(file, phdr->p_offset, SEEK_SET) != 0)
    {
        return false;
    }

    if(phdr->p_vaddr >= MEMSIZE || phdr->p_size > MEMSIZE - phdr->p_vaddr)
    {
        return false;
    }

    if(fread(memory + phdr->p_vaddr, 1, phdr->p_size, file) < phdr->p_size)
    {
        return false;
    }

    return true;
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void dump_phdrs (uint16_t numphdrs, elf_phdr_t *phdrs)
{
    printf("\n");
    for(int i = 0; i < numphdrs; i++)
    {
        bool can_execute = (phdrs[i].p_flags & FLAG_X) != 0;
        bool can_write = (phdrs[i].p_flags & FLAG_W) != 0;
        bool can_read = (phdrs[i].p_flags & FLAG_R) != 0;
        
        char flgs[] = "   ";

        if(can_execute)
        {
            flgs[2] = 'X';
        }
        if(can_write)
        {
            flgs[1] = 'W';
        }
        if(can_read)
        {
            flgs[0] = 'R';
        }

        
        char *type_str = phdrs[i].p_type == DATA ? "DATA" : phdrs[i].p_type == CODE ? "CODE" : phdrs[i].p_type == STACK ? "STACK" : phdrs[i].p_type == HEAP ? "HEAP" : "UNKNOWN";
        printf("  %-9.2d0x%-8.4x0x%-8.4x0x%-7.4x %-9.7s %-3s\n", i, phdrs[i].p_offset, phdrs[i].p_size, phdrs[i].p_vaddr, type_str, flgs);

    }
}

void dump_memory (byte_t *memory, uint16_t start, uint16_t end)
{
    if(!memory || start >= end)
    {
        return;
    }
    size_t start_base = (start >> 4) << 4;
    size_t len = (size_t)end - (size_t)start_base;
    for(size_t off = 0; off < len; off++)
    {
        
        size_t idx = (size_t)start_base + (size_t)off;
        
        if(off % 16 == 0)
        {
            printf("\n  %04x  ", (uint16_t) (idx));
        }
        if(off % 8 == 0 && off % 16 != 0)
        {
            printf(" ");
        }
        if(idx < start)
        {
            printf("  ");
        }
        else
        {
            printf("%02x", (unsigned)memory[idx]);
        }
        if(off + 1 == len)
        {
            continue;
        } else if(off % 16 == 15)
        {
            continue;
        } else
        {
            printf(" ");
        }
        
    }
}

