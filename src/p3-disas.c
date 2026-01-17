/*
 * CS 261 PA3: Mini-ELF disassembler
 *
 * Name: 
 */

#include "p3-disas.h"
#define MAX_BYTES 10

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

void print_addr_no_bytes(address_t pc)
{
    printf("  0x%03lx:", (unsigned long)pc);
    for (int i = 0; i < MAX_BYTES; i++) {
        printf("   ");
    }
    printf(" | ");
}

uint64_t get_immediate(byte_t *i)
{
    return ((uint64_t)i[0]) |
           ((uint64_t)i[1] << 8) |
           ((uint64_t)i[2] << 16) |
           ((uint64_t)i[3] << 24) |
           ((uint64_t)i[4] << 32) |
           ((uint64_t)i[5] << 40) |
           ((uint64_t)i[6] << 48) |
           ((uint64_t)i[7] << 56);
}

bool bytes_remaining(address_t pc, size_t need)
{
    if(pc >= MEMSIZE || need > (size_t)(MEMSIZE - pc))
    {
        return false;
    }
    return true;
}

byte_t high_order_nibble(byte_t b)
{
    return (b >> 4) & 0xF;
}

byte_t low_order_nibble(byte_t b)
{
    return b & 0xF;
}

bool valid_reg(byte_t r)
{
    return r <= 0xE;
}

bool is_none(byte_t r)
{
    return r == 0xF;
}

bool ifun_zero(byte_t f)
{
    return f == 0;
}

bool valid_ifun_cmov(byte_t f)
{
    return f <= 6;
}

bool valid_ifun_op(byte_t f)
{
    return f <= 3;
}

bool valid_ifun_jump(byte_t f)
{
    return f <= 6;
}

bool valid_ifun_iotrap(byte_t f)
{
    return f <= 5;
}

y86_inst_t fetch (y86_t *cpu, byte_t *memory)
{
    
    y86_inst_t ins;
    ins.icode = INVALID;
    ins.ifun.b = 0;
    ins.ra = NOREG;
    ins.rb = NOREG;
    ins.valC.v = 0;

    if(!cpu || !memory)
    {
        if(cpu)
        {
            cpu->stat = ADR;
        }
        return ins;
    }

    ins.valP = cpu->pc;

    address_t pc = cpu->pc;


    if(!bytes_remaining(pc, 1))
    {
        cpu->stat = ADR;
        ins.icode = INVALID;
        return ins;
    }

    byte_t opcode = memory[pc];
    byte_t icode = high_order_nibble(opcode);
    byte_t ifun = low_order_nibble(opcode);

    ins.icode = (y86_icode_t)icode;
    ins.ifun.b = ifun;
    ins.valP = pc + 1;
    byte_t regByte = 0;
    byte_t rA = 0;
    byte_t rB = 0;
    switch (ins.icode)
    {
    case HALT:
        if(!ifun_zero(ifun))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        ins.valP = pc + 1;
        cpu->stat = AOK;
        break;
    case NOP:
        if(!ifun_zero(ifun))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        ins.valP = pc + 1;
        cpu->stat = AOK;
        break;
    case CMOV:
        if(!valid_ifun_cmov(ifun))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        if(!bytes_remaining(pc, 2))
        {
            cpu->stat = ADR;
            ins.icode = INVALID;
            break;
        }
        ins.ifun.cmov = (y86_cmov_t)ifun;
        regByte = memory[pc + 1];
        rA = high_order_nibble(regByte);
        rB = low_order_nibble(regByte);
        
        if(!valid_reg(rA) || !valid_reg(rB))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        ins.ra = (y86_regnum_t)rA;
        ins.rb = (y86_regnum_t)rB;
        ins.valP = pc + 2;
        cpu->stat = AOK;
        break;
    case IRMOVQ:
        if(!ifun_zero(ifun))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        if(!bytes_remaining(pc, 10))
        {
            cpu->stat = ADR;
            ins.icode = INVALID;
            break;
        }
        regByte = memory[pc + 1];
        rA = high_order_nibble(regByte);
        rB = low_order_nibble(regByte);
        if(!is_none(rA) || !valid_reg(rB))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        ins.ra = (y86_regnum_t)rA;
        ins.rb = (y86_regnum_t)rB;
        ins.valC.v = (int64_t)get_immediate(&memory[pc + 2]);
        ins.valP = pc + 10;
        cpu->stat = AOK;
        break;
    case RMMOVQ:
        if(!ifun_zero(ifun))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        if(!bytes_remaining(pc, 10))
        {
            cpu->stat = ADR;
            ins.icode = INVALID;
            break;
        }
        regByte = memory[pc + 1];
        rA = high_order_nibble(regByte);
        rB = low_order_nibble(regByte);
        if(!valid_reg(rA) || (!valid_reg(rB) && !is_none(rB)))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        ins.ra = (y86_regnum_t)rA;
        ins.rb = (y86_regnum_t)rB;
        ins.valC.d = (int64_t)get_immediate(&memory[pc + 2]);
        ins.valP = pc + 10;
        cpu->stat = AOK;
        break;
    case MRMOVQ:
        if(!ifun_zero(ifun))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        if(!bytes_remaining(pc, 10))
        {
            cpu->stat = ADR;
            ins.icode = INVALID;
            break;
        }
        regByte = memory[pc + 1];
        rA = high_order_nibble(regByte);
        rB = low_order_nibble(regByte);
        if(!valid_reg(rA) || (!valid_reg(rB) && !is_none(rB)))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        ins.ra = (y86_regnum_t)rA;
        ins.rb = (y86_regnum_t)rB;
        ins.valC.d = (int64_t)get_immediate(&memory[pc + 2]);
        ins.valP = pc + 10;
        cpu->stat = AOK;
        break;
    case OPQ:
        if(!valid_ifun_op(ifun))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        if(!bytes_remaining(pc, 2))
        {
            cpu->stat = ADR;
            ins.icode = INVALID;
            break;
        }
        regByte = memory[pc + 1];
        rA = high_order_nibble(regByte);
        rB = low_order_nibble(regByte);
        if(!valid_reg(rA) || !valid_reg(rB))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        ins.ifun.op = (y86_op_t)ifun;
        ins.ra = (y86_regnum_t)rA;
        ins.rb = (y86_regnum_t)rB;
        ins.valP = pc + 2;
        cpu->stat = AOK;
        break;
    case JUMP:
        if(!valid_ifun_jump(ifun))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        if(!bytes_remaining(pc, 9))
        {
            cpu->stat = ADR;
            ins.icode = INVALID;
            break;
        }
        ins.ifun.jump = (y86_jump_t)ifun;
        ins.valC.dest = get_immediate(&memory[pc + 1]);
        ins.valP = pc + 9;
        cpu->stat = AOK;
        break;
    case CALL:
        if(!ifun_zero(ifun))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        if(!bytes_remaining(pc, 9))
        {
            cpu->stat = ADR;
            ins.icode = INVALID;
            break;
        }
        ins.valC.dest = get_immediate(&memory[pc + 1]);
        ins.valP = pc + 9;
        cpu->stat = AOK;
        break;
    case RET:
        if(!ifun_zero(ifun))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        ins.valP = pc + 1;
        cpu->stat = AOK;
        break;
    case PUSHQ:
        if(!ifun_zero(ifun))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        if(!bytes_remaining(pc, 2))
        {
            cpu->stat = ADR;
            ins.icode = INVALID;
            break;
        }
        regByte = memory[pc + 1];
        rA = high_order_nibble(regByte);
        rB = low_order_nibble(regByte);
        if(!valid_reg(rA) || !is_none(rB))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        ins.ra = (y86_regnum_t)rA;
        ins.rb = (y86_regnum_t)rB;
        ins.valP = pc + 2;
        cpu->stat = AOK;
        break;
    case POPQ:
        if(!ifun_zero(ifun))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            break;
        }
        if(!bytes_remaining(pc, 2))
        {
            cpu->stat = ADR;
            ins.icode = INVALID;
            break;
        }
        regByte = memory[pc + 1];
        rA = high_order_nibble(regByte);
        rB = low_order_nibble(regByte);
        if(!valid_reg(rA) || !is_none(rB))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            return ins;
        }
        ins.ra = (y86_regnum_t)rA;
        ins.rb = (y86_regnum_t)rB;
        ins.valP = pc + 2;
        cpu->stat = AOK;
        break;
    case IOTRAP:
        if(!valid_ifun_iotrap(ifun))
        {
            cpu->stat = INS;
            ins.icode = INVALID;
            return ins;
        }
        ins.ifun.trap = (y86_iotrap_t)ifun;
        ins.valP = pc + 1;
        cpu->stat = AOK;
        break;
    
    default:
        cpu->stat = INS;
        ins.icode = INVALID;
        break;
    }
    
    return ins;
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void disassemble (y86_inst_t *inst)
{
    /*HALT = 0, NOP, CMOV, IRMOVQ, RMMOVQ, MRMOVQ, OPQ, JUMP, CALL, RET, PUSHQ,
    POPQ, IOTRAP, INVALID*/
    char *reg_names[] = {
        "%rax", "%rcx", "%rdx", "%rbx",
        "%rsp", "%rbp", "%rsi", "%rdi",
        "%r8",  "%r9",  "%r10", "%r11",
        "%r12", "%r13", "%r14", "NOREG"
    };
    switch (inst->icode)
    {
    case HALT:
        printf("halt\n");
        return;
    case NOP:
        printf("nop\n");
        return;
    case CMOV:
        /*RRMOVQ = 0, CMOVLE, CMOVL, CMOVE, CMOVNE, CMOVGE, CMOVG, BADCMOV*/
        switch (inst->ifun.cmov)
        {
        case RRMOVQ:
            printf("rrmovq ");
            break;
        case CMOVLE:
            printf("cmovle ");
            break;
        case CMOVL:
            printf("cmovl ");
            break;
        case CMOVE:
            printf("cmove ");
            break;
        case CMOVNE:
            printf("cmovne ");
            break;
        case CMOVGE:
            printf("cmovge ");
            break;
        case CMOVG:
            printf("cmovg ");
            break;
        
        default:
            break;
        }
        printf("%s, %s\n", reg_names[inst->ra], reg_names[inst->rb]);
        break;
    case IRMOVQ:
        printf("irmovq 0x%lx, %s\n", (unsigned long)inst->valC.v, reg_names[inst->rb]);
        break;
    case RMMOVQ:
        if(inst->rb == 0xF)
        {
            printf("rmmovq %s, 0x%lx", reg_names[inst->ra], (unsigned long)inst->valC.d);
        } else
        {
            printf("rmmovq %s, 0x%lx(%s)", reg_names[inst->ra], (unsigned long)inst->valC.d, reg_names[inst->rb]);
        }
        printf("\n");
        break;
    case MRMOVQ:
        if(inst->rb == 0xF)
        {
            printf("mrmovq 0x%lx, %s", (unsigned long)inst->valC.d, reg_names[inst->ra]);
        } else
        {
            printf("mrmovq 0x%lx(%s), %s", (unsigned long)inst->valC.d, reg_names[inst->rb], reg_names[inst->ra]);
        }
        printf("\n");
        break;
    case OPQ:
        /*ADD = 0, SUB, AND, XOR, BADOP*/
        switch (inst->ifun.op)
        {
        case ADD:
            printf("addq ");
            break;
        case SUB:
            printf("subq ");
            break;
        case AND:
            printf("andq ");
            break;
        case XOR:
            printf("xorq ");
            break;
        
        default:
            break;
        }
        printf("%s, %s\n", reg_names[inst->ra], reg_names[inst->rb]);
        break;
    case JUMP:
        /*JMP = 0, JLE, JL, JE, JNE, JGE, JG, BADJUMP*/
        switch (inst->ifun.jump)
        {
        case JMP:
            printf("jmp ");
            break;
        case JLE:
            printf("jle ");
            break;
        case JL:
            printf("jl ");
            break;
        case JE:
            printf("je ");
            break;
        case JNE:
            printf("jne ");
            break;
        case JGE:
            printf("jge ");
            break;
        case JG:
            printf("jg ");
            break;
        
        default:
            break;
        }
        printf("0x%lx\n", (unsigned long)inst->valC.dest);
        break;
    case CALL:
        printf("call 0x%lx\n", (unsigned long)inst->valC.dest);
        break;
    case RET:
        printf("ret\n");
        return;
    case PUSHQ:
        printf("pushq %s\n", reg_names[inst->ra]);
        break;
    case POPQ:
        printf("popq %s\n", reg_names[inst->ra]);
        break;
    case IOTRAP:
        /*CHAROUT = 0, CHARIN, DECOUT, DECIN, STROUT, FLUSH, BADTRAP*/
        switch (inst->ifun.trap)
        {
        case CHAROUT:
            printf("iotrap 0\n");
            return;
        case CHARIN:
            printf("iotrap 1\n");
            return;
        case DECOUT:
            printf("iotrap 2\n");
            return;
        case DECIN:
            printf("iotrap 3\n");
            return;
        case STROUT:
            printf("iotrap 4\n");
            return;
        case FLUSH:
            printf("iotrap 5\n");
            return;
        
        default:
            break;
        }
        return;
    default:
        break;
    }

    /*RAX = 0, RCX, RDX, RBX, RSP, RBP, RSI, RDI,
    R8, R9, R10, R11, R12, R13, R14, NOREG*/
    return;
}

bool invalid_cpu(y86_t *cpu)
{
    return cpu->stat == INS || cpu->stat == ADR;
}

void disassemble_code (byte_t *memory, elf_phdr_t *phdr, elf_hdr_t *hdr)
{
    y86_t cpu;
    y86_inst_t ins;

    cpu.pc = phdr->p_vaddr;
    //address_t seg_start = phdr->p_vaddr;
    //address_t seg_end = phdr->p_vaddr + phdr->p_size;
    address_t entry = hdr->e_entry;
    cpu.stat = AOK;
    bool printed_start = false;

    //printf("Disassembly of executable contents:\n");
    print_addr_no_bytes(cpu.pc);
    printf(".pos 0x%03lx code\n", (unsigned long)cpu.pc);
    /*if(entry >= seg_start && entry < seg_end)
    {
        print_addr_no_bytes(entry);
        printf("_start:\n");
    }*/
    
    // printf("  %x:%*s| ", cpu.pc, 30, "");
    while(cpu.pc < phdr->p_vaddr + phdr->p_size)
    {
        if(!printed_start && entry == cpu.pc)
        {
            print_addr_no_bytes(cpu.pc);
            printf("_start:\n");
            printed_start = true;
        }

        ins = fetch(&cpu, memory);

        if(invalid_cpu(&cpu))
        {
            switch (cpu.stat)
            {
            case INS:
                printf("Invalid opcode: 0x%02x\n", memory[cpu.pc]);
                break;
            case ADR:
                break;
            default:
                break;
            }
            return;
        }
        size_t ins_len = ins.valP - cpu.pc;

        printf("  0x%03lx:", (unsigned long)cpu.pc);

        size_t i;
        for(i = 0; i < ins_len && i < MAX_BYTES; i++)
        {
            printf(" %02x", memory[cpu.pc + i]);
        }
        for(; i < MAX_BYTES; i++)
        {
            printf("   ");
        }

        printf(" |   ");
        disassemble(&ins);
        cpu.pc = ins.valP;
    }
}

void disassemble_data (byte_t *memory, elf_phdr_t *phdr)
{

    uint32_t start = phdr->p_vaddr;
    uint32_t end = start + phdr->p_size;

    //printf("Disassembly of data contents:\n");
    print_addr_no_bytes(start);
    printf(".pos 0x%03x data\n", start);

    for(uint32_t addr = start; addr + 8 <= end; addr += 8)
    {
        uint64_t val = 0;
        for(int i = 0; i < 8; i++)
        {
            val |= ((uint64_t)memory[addr + i]) << (8 * i);
        }
        printf("  0x%03x:", addr);

        int i;
        for(i = 0; i < 8; i++)
        {
            printf(" %02x", memory[addr + i]);
        }

        for(; i < MAX_BYTES; i++)
        {
            printf("   ");
        }

        printf(" |   .quad 0x%lx\n", (unsigned long)val);
    }

    

}

void disassemble_rodata (byte_t *memory, elf_phdr_t *phdr)
{

    uint32_t start = phdr->p_vaddr;
    uint32_t end = start + phdr->p_size;

    //printf("Disassembly of data contents:\n");
    print_addr_no_bytes(start);
    printf(".pos 0x%03x rodata\n", start);

    uint32_t addr = start;

    while(addr < end)
    {
        uint32_t str_start = addr;
        uint32_t len = 0;

        while(str_start + len < end && memory[str_start + len] != 0)
        {
            len++;
        }

        uint32_t total_bytes = len;
        if(str_start + len < end && memory[str_start + len] == 0)
        {
            total_bytes++;
        }

        char buffer[128];
        uint32_t copy_len = len;
        if(copy_len >= sizeof(buffer))
        {
            copy_len = sizeof(buffer) - 1;
        }

        for(uint32_t i = 0; i < copy_len; i++)
        {
            buffer[i] = (char)memory[str_start + i];
        }
        buffer[copy_len] = '\0';

        uint32_t line_addr = str_start;
        uint32_t remaining = total_bytes;
        bool first_line = true;

        while(remaining > 0)
        {
            uint32_t line_bytes = remaining > MAX_BYTES ? MAX_BYTES : remaining;

            printf("  0x%03x:", line_addr);

            uint32_t i = 0;

            for(; i < line_bytes; i++)
            {
                printf(" %02x", memory[line_addr + i]);
            }

            for(; i < MAX_BYTES; i++)
            {
                printf("   ");
            }

            printf(" |");

            if(first_line)
            {
                printf("   .string \"%s\"", buffer);
                first_line = false;
            } else
            {
                printf(" ");
            }

            printf("\n");

            remaining -= line_bytes;
            line_addr += line_bytes;
        }

        addr = str_start + total_bytes;
    }

}

