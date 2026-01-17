/*
 * CS 261 PA1: Mini-ELF header verifier
 *
 * Name: 
 */

#include "p1-check.h"
/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

bool read_header (FILE *file, elf_hdr_t *hdr)
{

    unsigned char byte[16];
    if(!(file && hdr) || fread(byte, 1, sizeof(byte), file) < sizeof(byte)){
        return false;
    }

    hdr->e_version = byte[0] | (byte[1] << 8);
    hdr->e_entry = byte[2] | (byte[3] << 8);
    hdr->e_phdr_start = byte[4] | (byte[5] << 8);
    hdr->e_num_phdr = byte[6] | (byte[7] << 8);
    hdr->e_symtab = byte[8] | (byte[9] << 8);
    hdr->e_strtab = byte[10] | (byte[11] << 8);
    hdr->magic = byte[12] | (byte[13] << 8) | (byte[14] << 16) | (byte[15] << 24);

    unsigned char *mag = (unsigned char *) &hdr->magic;
    if(!(mag[0] == 'E' && mag[1] == 'L' && mag[2] == 'F' && mag[3] == 0))
    {
        return false;
    }

    return true;
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void dump_header (elf_hdr_t *hdr)
{
    unsigned char *str = (unsigned char *)hdr;
    for(int i = 0; i < sizeof(elf_hdr_t); i++)
    {
        if(i == 8)
        {
            printf(" ");
        }
        printf("%02x", str[i]);
        if(i < sizeof(elf_hdr_t) - 1)
        {
            printf(" ");
        }
    }
    printf("\n");
    printf("Mini-ELF version %d\n", hdr->e_version);
    printf("Entry point 0x%x\n", hdr->e_entry);
    printf("There are %d program headers, starting at offset %d (0x%x)\n", hdr->e_num_phdr, hdr->e_phdr_start, hdr->e_phdr_start);
    if(hdr->e_symtab == 0)
    {
        printf("There is no symbol table present\n");
    } else
    {
        printf("There is a symbol table starting at offset %d (0x%x)\n", hdr->e_symtab, hdr->e_symtab);
    }
    if(hdr->e_strtab == 0)
    {
        printf("There is no string table present\n");
    } else
    {
        printf("There is a string table starting at offset %d (0x%x)\n", hdr->e_strtab, hdr->e_strtab);
    }
}
