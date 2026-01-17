/*
 * CS 261 PA4: Mini-ELF interpreter
 *
 * Name: 
 */

#include "p4-interp.h"

#define BUF_SIZE 100

void setAddCC(y86_t *cpu, y86_reg_t valA, y86_reg_t valB);
void setSubCC(y86_t *cpu, y86_reg_t valA, y86_reg_t valB);
void setAndCC(y86_t *cpu, y86_reg_t valA, y86_reg_t valB);
void setXorCC(y86_t *cpu, y86_reg_t valA, y86_reg_t valB);
#define MEM_OK(addr, size) ((addr) <= MEMSIZE - (size))

static char   outbuf[BUF_SIZE + 1];
static size_t outlen = 0;
/**********************************************************************
 *                       SELF-HELP FUNCTIONS
 *********************************************************************/

bool Cond(flag_t flags[], unsigned int ifun)
{
    int sf = 0;
    int of = 1;
    int zf = 2;
    switch (ifun)
    {
    case 0:
        return true;
    case 1:
        return (flags[sf] ^ flags[of]) || flags[zf];
    case 2:
        return flags[sf] ^ flags[of];
    case 3:
        return flags[zf];
    case 4:
        return !(flags[zf]);
    case 5:
        return !(flags[sf] ^ flags[of]);
    case 6:
        return !(flags[sf] ^ flags[of]) && !flags[zf];
    default:
        return false;
    }
}

void setCC(y86_t *cpu, y86_op_t op, y86_reg_t valA, y86_reg_t valB)
{
    switch(op) {
        case ADD: 
            setAddCC(cpu, valA, valB); 
            break;
        case SUB: 
            setSubCC(cpu, valA, valB); 
            break;
        case AND: 
            setAndCC(cpu, valA, valB); 
            break;
        case XOR: 
            setXorCC(cpu, valA, valB); 
            break;
        default: 
            break;
    }
}

void setAddCC(y86_t *cpu, y86_reg_t valA, y86_reg_t valB)
{
    y86_reg_t result = valB + valA;

    // Calculate first bit for a 0 or 1 (positive/negative) check
    uint64_t bitMask = (uint64_t)1 << 63;
    uint64_t sign_a = valA & bitMask;
    uint64_t sign_b = valB & bitMask;
    uint64_t sign_r = result & bitMask;

    // Overflow-Flag check
    cpu->of = (sign_a == sign_b) && (sign_a != sign_r);

    // Zero-Flag check
    cpu->zf = result == 0;

    // Sign-Flag check
    cpu->sf = (sign_r != 0);
    
}

void setSubCC(y86_t *cpu, y86_reg_t valA, y86_reg_t valB)
{
    y86_reg_t result = valB - valA;

    // Calculate first bit for a 0 or 1 (positive/negative) check
    uint64_t bitMask = (uint64_t)1 << 63;
    uint64_t sign_a = valA & bitMask;
    uint64_t sign_b = valB & bitMask;
    uint64_t sign_r = result & bitMask;

    // Overflow-Flag check
    cpu->of = (sign_b != sign_a) && (sign_b != sign_r);

    // Zero-Flag check
    cpu->zf = result == 0;

    // Sign-Flag check
    cpu->sf = (sign_r != 0);
}

void setAndCC(y86_t *cpu, y86_reg_t valA, y86_reg_t valB)
{
    y86_reg_t result = valB & valA;

    // Calculate first bit for a 0 or 1 (positive/negative) check
    uint64_t bitMask = (uint64_t)1 << 63;
    uint64_t sign_r = result & bitMask;

    // Overflow-Flag check
    cpu->of = false;

    // Zero-Flag check
    cpu->zf = result == 0;

    // Sign-Flag check
    cpu->sf = (sign_r != 0);
}

void setXorCC(y86_t *cpu, y86_reg_t valA, y86_reg_t valB)
{
    y86_reg_t result = valB ^ valA;

    // Calculate first bit for a 0 or 1 (positive/negative) check
    uint64_t bitMask = (uint64_t)1 << 63;
    uint64_t sign_r = result & bitMask;

    // Overflow-Flag check
    cpu->of = false;

    // Zero-Flag check
    cpu->zf = result == 0;

    // Sign-Flag check
    cpu->sf = (sign_r != 0);
}

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

y86_reg_t decode_execute (y86_t *cpu, y86_inst_t *inst, bool *cnd, y86_reg_t *valA)
{
    y86_reg_t valE = 0;
    y86_reg_t valB = 0;

    if (cpu == NULL) {
        return 0;
    }
    if (inst == NULL || cnd == NULL || valA == NULL) {
        cpu->stat = INS;
        return 0;
    }

    *cnd = false;

    if (inst->icode < HALT || inst->icode > IOTRAP || inst->icode == INVALID) {
        cpu->stat = INS;
        return 0;
    }

    /* Decode */
    switch (inst->icode)
    {
    case CMOV:
        if (inst->ifun.cmov < RRMOVQ || inst->ifun.cmov > CMOVG) {
            cpu->stat = INS;
            return 0;
        }
        *valA = cpu->reg[inst->ra];
        break;
    case RMMOVQ:
        *valA = cpu->reg[inst->ra];
        valB = cpu->reg[inst->rb];
        break;
    case MRMOVQ:
        valB = cpu->reg[inst->rb];
        break;
    case OPQ:
        if (inst->ifun.op < ADD || inst->ifun.op > XOR) {
            cpu->stat = INS;
            return 0;
        }
        *valA = cpu->reg[inst->ra];
        valB = cpu->reg[inst->rb];
        break;
    case CALL:
        valB = cpu->reg[RSP];
        break;
    case RET:
        *valA = cpu->reg[RSP];
        valB = cpu->reg[RSP];
        break;
    case PUSHQ:
        *valA = cpu->reg[inst->ra];
        valB = cpu->reg[RSP];
        break;
    case POPQ:
        *valA = cpu->reg[RSP];
        valB = cpu->reg[RSP];
        break;
    default:
        break;
    }

    /* Execute */

    /* FLAGS */
    flag_t sf = cpu->sf;
    flag_t of = cpu->of;
    flag_t zf = cpu->zf;
    flag_t flags[] = {sf, of, zf};

    switch (inst->icode)
    {
    case HALT:
        cpu->stat = HLT;
        break;
    case CMOV:
        if (inst->ifun.cmov < RRMOVQ || inst->ifun.cmov > CMOVG) {
            cpu->stat = INS;
            return 0;
        }
        valE = *valA;
        *cnd = Cond(flags, inst->ifun.b);
        break;
    case IRMOVQ:
        valE = inst->valC.v;
        break;
    case RMMOVQ:
        valE = valB + inst->valC.d;
        break;
    case MRMOVQ:
        valE = valB + inst->valC.d;
        break;
    case OPQ:
        if (inst->ifun.op < ADD || inst->ifun.op > XOR) {
            cpu->stat = INS;
            return 0;
        }
        switch (inst->ifun.op)
        {
        case ADD:
            valE = valB + *valA;
            break;
        case SUB:
            valE = valB - *valA;
            break;
        case AND:
            valE = valB & *valA;
            break;
        case XOR:
            valE = valB ^ *valA;
            break;
        default:
            break;
        }
        setCC(cpu, inst->ifun.op, *valA, valB);
        break;
    case JUMP:
        if (inst->ifun.jump < JMP || inst->ifun.jump > JG) {
            cpu->stat = INS;
            return 0;
        }
        *cnd = Cond(flags, inst->ifun.b);
        break;
    case CALL:
        valE = valB - 8;
        break;
    case RET:
        valE = valB + 8;
        break;
    case PUSHQ:
        valE = valB - 8;
        break;
    case POPQ:
        valE = valB + 8;
        break;
    default:
        break;
    }

    return valE;
}

void memory_wb_pc (y86_t *cpu, y86_inst_t *inst, byte_t *memory,
        bool cnd, y86_reg_t valA, y86_reg_t valE)
{

    if (cpu == NULL) {
        return;
    }
    if (inst == NULL || memory == NULL) {
        cpu->stat = INS;
        return;
    }

    /* Memory */
    uint64_t valM = 0;
    switch (inst->icode)
    {
    case RMMOVQ:
        if (!MEM_OK(valE, sizeof(uint64_t))) {
            cpu->stat = ADR;
            return;
        }
        memcpy(&memory[valE], &valA, sizeof(valA));
        //memory[valE] = valA;
        break;
    case MRMOVQ:
        if (!MEM_OK(valE, sizeof(uint64_t))) {
            cpu->stat = ADR;
            return;
        }
        memcpy(&valM, &memory[valE], sizeof(uint64_t));
        //valM = memory[valE]
        break;
    case CALL:
        if (!MEM_OK(valE, sizeof(uint64_t))) {
            cpu->stat = ADR;
            return;
        }
        memcpy(&memory[valE], &inst->valP, sizeof(valA));
        break;
    case RET:
        if (!MEM_OK(valA, sizeof(uint64_t))) {
            cpu->stat = ADR;
            return;
        }
        memcpy(&valM, &memory[valA], sizeof(uint64_t));
        //memcpy(&memory[valE], valA, sizeof(valA)); PUSH?
        break;
    case PUSHQ:
        if (!MEM_OK(valE, sizeof(uint64_t))) {
            cpu->stat = ADR;
            return;
        }
        memcpy(&memory[valE], &valA, sizeof(valA));
        break;
    case POPQ:
        if (!MEM_OK(valA, sizeof(uint64_t))) {
            cpu->stat = ADR;
            return;
        }
        memcpy(&valM, &memory[valA], sizeof(uint64_t));
        break;
    case IOTRAP:
        switch (inst->ifun.trap) 
        {

        case CHAROUT: {   // trap 0: write one char from memory at %rsi
            address_t addr = cpu->reg[RSI];
            byte_t ch = memory[addr];

            if (outlen + 1 >= BUF_SIZE) goto io_fail;
            outbuf[outlen++] = (char)ch;
            outbuf[outlen] = '\0';
            break;
        }

        case CHARIN: {    // trap 1: read one char into memory at %rdi
            address_t addr = cpu->reg[RDI];
            int ch = getchar();          // or scanf(" %c", &c)
            if (ch == EOF) goto io_fail;
            memory[addr] = (byte_t)ch;
            break;
        }

        case DECOUT: {    // trap 2: write 64-bit int in decimal from memory at %rsi
            address_t addr = cpu->reg[RSI];
            uint64_t val;
            memcpy(&val, &memory[addr], sizeof(uint64_t));

            char tmp[32];
            int n = snprintf(tmp, sizeof(tmp), "%lld", (long long)val);
            if (n < 0) goto io_fail;
            if ((size_t)n > BUF_SIZE - outlen) goto io_fail;

            memcpy(outbuf + outlen, tmp, (size_t)n);
            outlen += (size_t)n;
            outbuf[outlen] = '\0';
            break;
        }

        case DECIN: {     // trap 3: read 64-bit int into memory at %rdi
            address_t addr = cpu->reg[RDI];
            long long val;

            if (scanf("%lld", &val) != 1) goto io_fail;
            memcpy(&memory[addr], &val, sizeof(uint64_t));
            break;
        }

        case STROUT: {    // trap 4: write null-terminated string from memory at %rsi
            address_t addr = cpu->reg[RSI];

            while (1) {
                byte_t ch = memory[addr++];
                if (ch == '\0') break;

                if (outlen + 1 >= BUF_SIZE) goto io_fail;
                outbuf[outlen++] = (char)ch;
            }
            outbuf[outlen] = '\0';
            break;
        }

        case FLUSH: {     // trap 5: flush buffer to stdout
            outbuf[outlen] = '\0';
            printf("%s", outbuf);
            outlen = 0;
            outbuf[0] = '\0';
            break;
        }

        default:
            goto io_fail;
        }
        break;
    default:
        break;
    }

    /* Write Back */
    switch (inst->icode)
    {
    case CMOV:
        if(cnd)
        {
            cpu->reg[inst->rb] = valE;
        }
        break;
    case IRMOVQ:
        cpu->reg[inst->rb] = valE;
        break;
    case MRMOVQ:
        cpu->reg[inst->ra] = valM;
        break;
    case OPQ:
        cpu->reg[inst->rb] = valE;
        break;
    case CALL:
        cpu->reg[RSP] = valE;
        break;
    case RET:
        cpu->reg[RSP] = valE;
        break;
    case PUSHQ:
        cpu->reg[RSP] = valE;
        break;
    case POPQ:
        cpu->reg[RSP] = valE;
        cpu->reg[inst->ra] = valM;
        break;
    default:
        break;
    }

    /* Program Counter */
    flag_t of = cpu->of;
    flag_t zf = cpu->zf;
    flag_t sf = cpu->sf;
    switch (inst->icode)
    {
    case JUMP:
        switch (inst->ifun.jump)
        {
        case JMP:
            cpu->pc = inst->valC.dest;
            break;
        case JLE:
            cpu->pc = (zf || sf != of) ? inst->valC.dest: inst->valP;
            break;
        case JL:
            cpu->pc = (sf != of) ? inst->valC.dest: inst->valP;
            break;
        case JE:
            cpu->pc = zf ? inst->valC.dest: inst->valP;
            break;
        case JNE:
            cpu->pc = !zf ? inst->valC.dest: inst->valP;
            break;
        case JGE:
            cpu->pc = (sf == of) ? inst->valC.dest: inst->valP;
            break;
        case JG:
            cpu->pc = (!zf && sf == of) ? inst->valC.dest: inst->valP;
            break;
        
        default:
            break;
        }
        break;
    case CALL:
        cpu->pc = inst->valC.dest;
        break;
    case RET:
        cpu->pc = valM;
        break;
    default:
        cpu->pc = inst->valP;
    }

    return;
    io_fail:
        printf("I/O Error\n");
        cpu->stat = HLT;
        cpu->pc = inst->valP;




    /*switch (inst->icode)
    {
    case HALT:
        break;
    case NOP:
        cpu->pc = inst->valP;
        break;
    case CMOV:
        cpu->pc = inst->valP;
        break;
    case IRMOVQ:
        break;
    case RMMOVQ:
        break;
    case MRMOVQ:
        break;
    case OPQ:
        break;
    case JUMP:
        break;
    case CALL:
        break;
    case RET:
        break;
    case PUSHQ:
        break;
    case POPQ:
        break;
    default:
        break;
    } */
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void dump_cpu_state (y86_t *cpu)
{
    char *stat_str;

    switch (cpu->stat) {
    case AOK: stat_str = "AOK"; break;
    case HLT: stat_str = "HLT"; break;
    case ADR: stat_str = "ADR"; break;
    case INS: stat_str = "INS"; break;
    default:  stat_str = "???"; break;
    }

    printf("Y86 CPU state:\n");
    printf("    PC: %016llx   flags: Z%d S%d O%d     %s\n",
           (unsigned long long)cpu->pc,
           cpu->zf ? 1 : 0,
           cpu->sf ? 1 : 0,
           cpu->of ? 1 : 0,
           stat_str);

    printf("  %%rax: %016llx    %%rcx: %016llx\n",
           (unsigned long long)cpu->reg[RAX],
           (unsigned long long)cpu->reg[RCX]);

    printf("  %%rdx: %016llx    %%rbx: %016llx\n",
           (unsigned long long)cpu->reg[RDX],
           (unsigned long long)cpu->reg[RBX]);

    printf("  %%rsp: %016llx    %%rbp: %016llx\n",
           (unsigned long long)cpu->reg[RSP],
           (unsigned long long)cpu->reg[RBP]);

    printf("  %%rsi: %016llx    %%rdi: %016llx\n",
           (unsigned long long)cpu->reg[RSI],
           (unsigned long long)cpu->reg[RDI]);

    printf("   %%r8: %016llx     %%r9: %016llx\n",
           (unsigned long long)cpu->reg[R8],
           (unsigned long long)cpu->reg[R9]);

    printf("  %%r10: %016llx    %%r11: %016llx\n",
           (unsigned long long)cpu->reg[R10],
           (unsigned long long)cpu->reg[R11]);

    printf("  %%r12: %016llx    %%r13: %016llx\n",
           (unsigned long long)cpu->reg[R12],
           (unsigned long long)cpu->reg[R13]);

    printf("  %%r14: %016llx\n",
           (unsigned long long)cpu->reg[R14]);
}

