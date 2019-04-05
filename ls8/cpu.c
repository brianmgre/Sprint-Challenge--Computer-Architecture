#include "cpu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DATA_LEN 6

unsigned char cpu_ram_read(struct cpu *cpu, unsigned char address)
{
  return cpu->ram[address];
}

void cpu_ram_write(struct cpu *cpu, unsigned char address, unsigned char value)
{
  cpu->ram[address] = value;
};

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */
void cpu_load(struct cpu *cpu, char *file)
{
  FILE *fp = fopen(file, "r");
  char line[1024];

  if (fp == NULL)
  {
    fprintf(stderr, "No file.\n");
    exit(1);
  }

  int address = 0;

  while (fgets(line, sizeof(line), fp) != NULL)
  {
    char *endptr;
    unsigned char value = strtoul(line, &endptr, 2);

    if (endptr == line)
    {
      continue;
    }

    cpu->ram[address++] = value;
  }
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
  switch (op)
  {
  case ALU_MUL:
    cpu->registers[regA] = (cpu->registers[regA] * cpu->registers[regB]) & 0xFF;
    break;

  case ALU_ADD:
    cpu->registers[regA] = (cpu->registers[regA] + cpu->registers[regB]) & 0xFF;
    break;

  case ALU_CMP:
    if (cpu->registers[regA] == cpu->registers[regB])
    {
      cpu->E = 1;
      break;
    }
    else if (cpu->registers[regA] < cpu->registers[regB])
    {
      cpu->L = 1;
      break;
    }
    else
    {
      cpu->G = 1;
      break;
    }
    break;

  default:
    printf("error\n");
  }
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  int running = 1; // True until we get a HLT instruction

  while (running)
  {
    unsigned char ir = cpu_ram_read(cpu, cpu->PC);
    unsigned char operandA;
    unsigned char operandB;
    int next_line = 1;

    if (ir & 0x80)
    {
      operandA = cpu_ram_read(cpu, cpu->PC + 1);
      operandB = cpu_ram_read(cpu, cpu->PC + 2);
      next_line = 3;
    }
    else if (ir & 0x40)
    {
      operandA = cpu_ram_read(cpu, cpu->PC + 1);
      next_line = 2;
    }

    switch (ir)
    {

    case ADD:
      alu(cpu, ALU_ADD, operandA, operandB);
      break;

    case MUL:
      alu(cpu, ALU_MUL, operandA, operandB);
      break;

    case LDI:
      cpu->registers[operandA] = operandB;
      break;

    case PRN:
      printf("%d\n", cpu->registers[operandA]);
      break;

    case PUSH:
      cpu_ram_write(cpu, --cpu->registers[7], cpu->registers[operandA]);
      break;

    case POP:
      cpu->registers[operandA] = cpu_ram_read(cpu, cpu->registers[7]++);
      break;

    case CALL:
      cpu->ram[--cpu->registers[7]] = cpu->PC + next_line;
      cpu->PC = cpu->registers[operandA];
      continue;

    case RET:
      cpu->PC = cpu->ram[cpu->registers[7]++];
      continue;

    case CMP:
      alu(cpu, ALU_CMP, operandA, operandB);
      break;

    case HLT:
      running = 0;
      break;

    default:
      break;
    }
    cpu->PC = cpu->PC + next_line;
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  cpu->PC = 0;
  cpu->FL = 0;
  cpu->E = 0;
  cpu->L = 0;
  cpu->G = 0;
  memset(cpu->ram, 0, 8 * sizeof(unsigned char));
  memset(cpu->registers, 0, 256 * sizeof(unsigned char));
  cpu->registers[7] = 0xF4;
}
