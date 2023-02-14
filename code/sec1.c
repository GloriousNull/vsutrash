
#include "basic.c"

#define WIN_CONSOLE
#include "win.c"

#include "stdio.h"

static
u16 u16_cycle_left_shift(u16 value, u8 offset)
{
  u16 result = (value << offset) | (value >> (16 - offset));
  return(result);
}

static
u16 u16_cycle_right_shift(u16 value, u8 offset)
{
  u16 result = (value >> offset) | (value << (16 - offset));
  return(result);
}

static
u32 u32_cycle_left_shift(u32 value, u8 offset)
{
  u32 result = (value << offset) | (value >> (32 - offset));
  return(result);
}

static
u32 u32_cycle_right_shift(u32 value, u8 offset)
{
  u32 result = (value >> offset) | (value << (32 - offset));
  return(result);
}

static
u64 u64_cycle_left_shift(u64 value, u8 offset)
{
  u64 result = (value << offset) | (value >> (64 - offset));
  return(result);
}

static
u64 u64_cycle_right_shift(u64 value, u8 offset)
{
  u64 result = (value >> offset) | (value << (64 - offset));
  return(result);
}

#define KEY 0xF7B1CAF0D1EB6C3B
#define ENCODE_ITERATIONS 2

static
u16 generate_key_4branches(u64 iteration)
{
  u16 result = (u16)u64_cycle_right_shift(KEY, iteration * 8);
  return(result);
}

static
u32 generate_key_2branches(u64 iteration)
{
  u32 result = (u32)u64_cycle_right_shift(KEY, iteration * 8);
  return(result);
}

static
u32 transform_2branches(u32 half, u32 key)
{
  u32 transformed_half = u32_cycle_right_shift(half, 7);
  u32 transformed_key = u32_cycle_left_shift(key, 9) | half;
  u32 result = transformed_half ^ transformed_key;
  return(result);
}

static
u16 transform_4branches(u16 part, u16 key)
{
  u16 transformed_part = u16_cycle_right_shift(part, 7);
  u16 transformed_key = u16_cycle_left_shift(key, 9) | part;
  u16 result = transformed_part ^ transformed_key;
  return(result);
}

static
u64 encode_block_4branches(u64 block)
{
  // little-endian aware division
  u64 mask = ~0ull >> 48;
  u16 part_0 = (u16)(block & mask);
  mask <<= 16;
  u16 part_1 = (u16)((block & mask) >> 16);
  mask <<= 16;
  u16 part_2 = (u16)((block & mask) >> 32);
  mask <<= 16;
  u16 part_3 = (u16)((block & mask) >> 48);
    
  for(u32 it = 0; it < ENCODE_ITERATIONS - 1; ++it)
  {
    u16 key = generate_key_4branches(it);

    u16 new_part_3 = part_0;
    u16 new_part_0 = part_1 ^ transform_4branches(part_0, key);

    part_0 = new_part_0;
    part_1 = part_2;
    part_2 = part_3;
    part_3 = new_part_3;
  }

  u16 key = generate_key_4branches(ENCODE_ITERATIONS - 1);
  part_1 ^= transform_4branches(part_0, key);
  
  u64 encoded_block = (u64)part_0 | ((u64)part_1 << 16) | ((u64)part_2 << 32) | ((u64)part_3 << 48);
  return(encoded_block);
}

static
u64 decode_block_4branches(u64 block)
{
  // little-endian aware division
  u64 mask = ~0ull >> 48;
  u16 part_0 = (u16)(block & mask);
  mask <<= 16;
  u16 part_1 = (u16)((block & mask) >> 16);
  mask <<= 16;
  u16 part_2 = (u16)((block & mask) >> 32);
  mask <<= 16;
w  u16 part_3 = (u16)((block & mask) >> 48);
    
  for(u32 it = ENCODE_ITERATIONS - 1; it > 0; --it)
  {
    u16 key = generate_key_4branches(it);
    
    u16 new_part_3 = part_0;
    u16 transformed_part_1 = part_1 ^ transform_4branches(part_0, key);

    part_0 = transformed_part_1;
    part_1 = part_2;
    part_2 = part_3;
    part_3 = new_part_3;
  }

  u16 key = generate_key_4branches(0);
  part_1 ^= transform_4branches(part_0, key);
  
  u64 decoded_block = (u64)part_0 | ((u64)part_1 << 16) | ((u64)part_2 << 32) | ((u64)part_3 << 48);
  return(decoded_block);
}

static
void encode_4branches(u8 const * input, u8 * output, u64 size)
{
  u64 const * blocks = (u64 *)input;
  u64 * encoded_blocks = (u64 *)output;
  u64 blocks_size = size / 8;

  for(u64 blocks_it = 0; blocks_it < blocks_size; ++blocks_it)
  {
    u64 block = blocks[blocks_it];

    u64 encoded_block = encode_block_4branches(block);
    encoded_blocks[blocks_it] = encoded_block;
  }  
}

static
void decode_4branches(u8 const * input, u8 * output, u64 size)
{
  u64 const * blocks = (u64 *)input;
  u64 * decoded_blocks = (u64 *)output;
  u64 blocks_size = size / 8;

  for(u64 blocks_it = 0; blocks_it < blocks_size; ++blocks_it)
  {
    u64 block = blocks[blocks_it];

    u64 decoded_block = decode_block_4branches(block);
    decoded_blocks[blocks_it] = decoded_block;
  }  
}

static
void encode_2branches(u8 const * input, u8 * output, u64 size)
{
  u64 const * blocks = (u64 *)input;
  u64 * encoded_blocks = (u64 *)output;
  u64 blocks_size = size / 8;

  for(u64 blocks_it = 0; blocks_it < blocks_size; ++blocks_it)
  {
    u64 block = blocks[blocks_it];

    u32 left = (u32)(block & ((~0ull) >> 32));
    u32 right = (u32)(block >> 32);

    for(u32 it = 0; it < ENCODE_ITERATIONS - 1; ++it)
    {
      u32 key = generate_key_2branches(it);

      u32 new_right = right ^ transform_2branches(left, key);

      right = left;
      left = new_right;
    }

    u32 key = generate_key_2branches(ENCODE_ITERATIONS - 1);

    u32 new_right = right ^ transform_2branches(left, key);
    right = new_right;

    u64 encoded_block = (u64)left | ((u64)right << 32);
    encoded_blocks[blocks_it] = encoded_block;
  }
}

static
void decode_2branches(u8 const * input, u8 * output, u64 size)
{
  u64 const * encoded_blocks = (u64 *)input;
  u64 * decoded_blocks = (u64 *)output;
  u64 blocks_size = size / 8;

  for(u64 blocks_it = 0; blocks_it < blocks_size; ++blocks_it)
  {
    u64 block = encoded_blocks[blocks_it];

    u32 left = (u32)(block & ((~0ull) >> 32));
    u32 right = (u32)(block >> 32);

    for(u32 it = ENCODE_ITERATIONS - 1; it > 0; --it)
    {
      u32 key = generate_key_2branches(it);

      u32 transformed_right = right ^ transform_2branches(left, key);

      right = left;
      left = transformed_right;
    }

    u32 key = generate_key_2branches(0);

    u32 transformed_right = right ^ transform_2branches(left, key);
    right = transformed_right;

    u64 decoded_block = (u64)left | ((u64)right << 32);
    decoded_blocks[blocks_it] = decoded_block;
  }  
}

s32 main(void)
{
  u32 memory_size = mebibytes(50);
  memory_arena storage =
  {
    .base = os_allocate_memory(memory_size),
    .size = memory_size,
  };

  buffer input = read_entire_file_to_arena(&storage, "../data/sec1.data");

  if(!input.size)
    return(-1);
  
  u64 data_size = next_multiple(input.size, 8);
  u8 * data = push_size(&storage, data_size, 32);
  u8 * encoded_data = push_size(&storage, data_size, 32);
  u8 * decoded_data = push_size(&storage, data_size, 32);

  copy_32bytes_aligned_memory(data, input.memory, input.size);

  os_print("ORIGINAL:\n\n");
  os_print_buffer(data, data_size);

  encode_2branches(data, encoded_data, data_size);
  os_print("\n\nENCODED 2BRACNHES:\n\n");

  for(u32 it = 0; it < data_size / 8; ++it)
  {
    u64 value = ((u64 *)encoded_data)[it];
    print("%u ", value);
  }
  
  decode_2branches(encoded_data, decoded_data, data_size);
  os_print("\n\nDECODED 2BRANCHES:\n\n");
  os_print_buffer(decoded_data, data_size);

  encode_4branches(data, encoded_data, data_size);
  os_print("\n\nENCODED 4BRACNHES:\n\n");

  for(u32 it = 0; it < data_size / 8; ++it)
  {
    u64 value = ((u64 *)encoded_data)[it];
    print("%u ", value);
  }
  
  decode_4branches(encoded_data, decoded_data, data_size);
  os_print("\n\nDECODED 4BRANCHES:\n\n");
  os_print_buffer(decoded_data, data_size);

  
  return(0);
}
