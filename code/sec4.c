
#include "basic.c"

#define WIN_CONSOLE
#include "win.c"

// variant 16

static
u64 mod_pow(u64 base, u64 exp, u64 modulus)
{
  base %= modulus;
  u64 result = 1;
  while (exp > 0)
  {
    if (exp & 1)
      result = (result * base) % modulus;
    
    base = (base * base) % modulus;
    exp >>= 1;
  }
  
  return(result);
}

static
void parse_overthinked_stuff(char * output, u64 * input, u64 input_size)
{
  char ascii_buffer[10];
  u64 output_it = 0;
  
  for(u64 it = 0; it < input_size; ++it)
  {
    u64 buffer_it = 9;
    u64 block = input[it];

    while(block)
    {
      char ascii = (char)(block % 100);
      ascii_buffer[buffer_it] = ascii;
      --buffer_it;
      block /= 100;
    }

    for(u64 ascii_it = buffer_it + 1; ascii_it < 10; ++ascii_it)
    {
      output[output_it] = ascii_buffer[ascii_it];
      ++output_it;
    }
  }
}

s32 main(void)
{
  u64 memory_size = mebibytes(10);
  memory_arena storage =
  {
    .base = os_allocate_memory(memory_size),
    .size = memory_size,
  };
  
  u64 n = 1814346979090559;
  u64 d = 1662573456022207;

  u64 cypher[] =
  {
    1021711154464307,
    598437030846800,
    186303081265080,
    787432302283150,
    809936855643157,
    1584051289686451,
  };

  u64 messages[] =
  {
    80767972797332,
    82656679677273,
    73327685677283,
    72693280767972,
    79717932786567,
    72657678737565,
  };

  u64 cypher_size = array_size(cypher);

  for(u64 it = 0; it < cypher_size; ++it)
  {
    cypher[it] = mod_pow(cypher[it], d, n);
  }

  u32 message_max_size = 100000;
  char * message = push_size(&storage, message_max_size, 32);

  parse_overthinked_stuff(message, messages, cypher_size);
  
  os_print((char *)message);
  
  return(0);
}
