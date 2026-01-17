/*
 * CS 261: Main driver
 *
 * Name: 
 */

#include "p1-check.h"
#include "p2-load.h"
#include "p3-disas.h"
#include "p4-interp.h"

/*
 * helper function for printing help text
 */
void usage (char **argv)
{
    printf("Usage: %s <option(s)> mini-elf-file\n", argv[0]);
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
    printf("  -a      Show all with brief memory\n");
    printf("  -f      Show all with full memory\n");
    printf("  -s      Show the program headers\n");
    printf("  -m      Show the memory contents (brief)\n");
    printf("  -M      Show the memory contents (full)\n");
    printf("  -d      Disassemble code contents\n");
    printf("  -D      Disassemble data contents\n");
    printf("  -e      Execute program\n");
    printf("  -E      Execute program (trace mode)\n");
}

int main (int argc, char **argv)
{
    int a = 0;
    bool print_header = false;
    bool show_phdrs = false;
    bool show_mem_brief = false;
    bool show_mem_full = false;
    bool dis_code = false;
    bool dis_data = false;
    bool exec = false;
    bool exec_trace = false;
    while((a = getopt(argc, argv, "hHafsmMdDeE")) != -1)
    {
        switch (a)
        {
        case 'h':
                usage(argv);
                return EXIT_SUCCESS;
                break;
            case 'H':
                print_header = true;
                break;
        case 'a':
            print_header = true;
            show_phdrs = true;
            show_mem_brief = true;
            break;
        case 'f':
            print_header = true;
            show_phdrs = true;
            show_mem_full = true;
            break;
        case 's':
            show_phdrs = true;
            break;
        case 'm':
            show_mem_brief = true;
            break;
        case 'M':
            show_mem_full = true;
            break;
        case 'd':
            dis_code = true;
            break;
        case 'D':
            dis_data = true;
            break;
        case 'e':
            exec = true;
            break;
        case 'E':
            exec_trace = true;
            show_mem_full = true;
            break;
        default:
                usage(argv);
                return EXIT_FAILURE;
                break;
        }
    }

    if(argc - 1 > optind || optind >= argc)
    {
        usage(argv);
        return EXIT_FAILURE;
    }
    if((show_mem_brief && show_mem_full) || (exec && exec_trace))
    {
        usage(argv);
        return EXIT_FAILURE;
    }
    elf_hdr_t e;
    FILE *f = fopen(argv[optind], "rb");
    if(f)
    {
        if(!read_header(f, &e))
        {
            printf("Failed to read file\n");
            fclose(f);
            return EXIT_FAILURE;
        }
        size_t num_phdrs = e.e_num_phdr;
        elf_phdr_t phdrs[num_phdrs];
        byte_t *mem_space = (byte_t *) calloc(MEMSIZE, 1);
        for(int i = 0; i < num_phdrs; i++)
        {
            if(!read_phdr(f, e.e_phdr_start + (i * 20), &phdrs[i])){
                printf("Failed to read file\n");
                fclose(f);
                free(mem_space);
                return EXIT_FAILURE;
            }
            if(!load_segment(f, mem_space, &phdrs[i]))
            {
                printf("Failed to read file\n");
                fclose(f);
                free(mem_space);
                return EXIT_FAILURE;
            }
        }
        fclose(f);
        if(print_header)
        {
            dump_header(&e);
        }
        if(show_phdrs)
        {
       /*printf(" %-9s %-9s %-9s %-9s %-9s %-3s",
       "Segment", "Offset", "Size", "VirtAddr", "Type", "Flags");*/
            dump_phdrs(num_phdrs, phdrs);
        }
        if(show_mem_brief)
        {
            for(int i = 0; i < num_phdrs; i++)
            {
                size_t start = phdrs[i].p_vaddr;
                size_t end = start + phdrs[i].p_size;
                //printf("Contents of memory from %04zx to %04zx:", start, end);

                dump_memory(mem_space, start, end);
                //printf("\n");
            }
            
        }
        if(exec_trace)
        {
            y86_t cpu;
            memset(&cpu, 0, sizeof(cpu));
            bool cnd;
            y86_reg_t valA, valE;

            cpu.stat = AOK;
            cpu.pc = e.e_entry;
            printf("Beginning execution at 0x%04x\n", (unsigned int)cpu.pc);
            dump_cpu_state(&cpu);
            printf("\n");
            int exec_count = 0;
            while(cpu.stat == AOK)
            {
                y86_inst_t ins = fetch(&cpu, mem_space);
                 if (cpu.stat == INS) {
                    printf("Invalid instruction at 0x%04llx\n",
                        (unsigned long long)cpu.pc);
                    dump_cpu_state(&cpu);
                    break;
        }
                if(cpu.stat != AOK) break;
                printf("Executing: ");
                disassemble(&ins);
                printf("\n");
                valE = decode_execute(&cpu, &ins, &cnd, &valA);
                memory_wb_pc(&cpu, &ins, mem_space, cnd, valA, valE);
                dump_cpu_state(&cpu);
                if(cpu.stat == AOK)
                {
                    printf("\n");
                }
                // printf("\n");
                exec_count++;
            }
            printf("Total execution count: %d\n\n", exec_count);
        }
        if(show_mem_full)
        {
            //printf("Contents of memory from %04x to %04x:", 0x0000, MEMSIZE);
            dump_memory(mem_space, 0, MEMSIZE);
            //printf("\n");
        }
        if (dis_code)
        {
            printf("Disassembly of executable contents:\n");
            for (size_t i = 0; i < num_phdrs; i++)
            {
                if (phdrs[i].p_type == CODE)
                {
                    disassemble_code(mem_space, &phdrs[i], &e);
                    printf("\n");
                }
            }
        }
        if (dis_data)
        {
            printf("Disassembly of data contents:\n");
            for (size_t i = 0; i < num_phdrs; i++)
            {
                if (phdrs[i].p_type == DATA)
                {
                    if (phdrs[i].p_flags & 0x2)
                    {
                        disassemble_data(mem_space, &phdrs[i]);
                    } else
                    {
                        disassemble_rodata(mem_space, &phdrs[i]);
                    }
                    printf("\n");
                }
            }
        }
        if(exec)
        {
            y86_t cpu;
            memset(&cpu, 0, sizeof(cpu));
            bool cnd;
            y86_reg_t valA, valE;

            cpu.stat = AOK;
            cpu.pc = e.e_entry;
            printf("Beginning execution at 0x%04x\n", (unsigned int)cpu.pc);
            int exec_count = 0;
            while(cpu.stat == AOK)
            {
                y86_inst_t ins = fetch(&cpu, mem_space);
                if(cpu.stat != AOK) break;
                valE = decode_execute(&cpu, &ins, &cnd, &valA);
                memory_wb_pc(&cpu, &ins, mem_space, cnd, valA, valE);
                exec_count++;
            }
            dump_cpu_state(&cpu);
            printf("Total execution count: %d\n", exec_count);
        }
        
        free(mem_space);

    } else
    {
        printf("Failed to read file\n");
        //fclose(f);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

