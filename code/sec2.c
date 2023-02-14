
#include "basic.c"

#define WIN_CONSOLE
#include "win.c"

#include "stdio.h"

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

#define KEY 0xF0B1AAF0B1ABABAB
#define VECTOR_KEY 0x1984AA451CBACAB
#define ENCODE_ITERATIONS 8

static
u32 generate_key(u64 iteration)
{
  u32 result = (u32)u64_cycle_right_shift(KEY, iteration * 8);
  return(result);
}

static
u32 transform(u32 half, u32 key)
{
  u32 transformed_half = u32_cycle_right_shift(half, 7);
  u32 transformed_key = u32_cycle_left_shift(key, 9) | half;
  u32 result = transformed_half ^ transformed_key;
  return(result);
}

static
u64 encode_block(u64 block)
{
  u32 left = (u32)(block & ((~0ull) >> 32));
  u32 right = (u32)(block >> 32);

  for(u32 it = 0; it < ENCODE_ITERATIONS - 1; ++it)
  {
    u32 key = generate_key(it);

    u32 new_left = left;
    u32 new_right = right ^ transform(left, key);

    left = new_right;
    right = new_left;
  }

  u32 key = generate_key(ENCODE_ITERATIONS - 1);

  right ^= transform(left, key);

  u64 encoded_block = (u64)left | ((u64)right << 32);
  return(encoded_block);
}

static
u64 decode_block(u64 block)
{
  u32 left = (u32)(block & ((~0ull) >> 32));
  u32 right = (u32)(block >> 32);

  for(u32 it = ENCODE_ITERATIONS - 1; it > 0; --it)
  {
    u32 key = generate_key(it);

    u32 transformed_right = right ^ transform(left, key);

    right = left;
    left = transformed_right;
  }

  u32 key = generate_key(0);

  right ^= transform(left, key);

  u64 decoded_block = (u64)left | ((u64)right << 32);
  return(decoded_block);
}

static
void ofb_encode(u8 const * input, u8 * output, u64 size) // output feedback
{
  assert(size % 8 == 0);
  u64 const * blocks = (u64 *)input;
  u64 * encoded_blocks = (u64 *)output;
  u64 blocks_size = size / 8;

  u64 additional_key = VECTOR_KEY;
  
  for(u64 blocks_it = 0; blocks_it < blocks_size; ++blocks_it)
  {
    additional_key = encode_block(additional_key);
    u64 block = blocks[blocks_it];

    u64 encoded_block = additional_key ^ block;
    encoded_blocks[blocks_it] = encoded_block;
  }
}

static
void ofb_decode(u8 const * input, u8 * output, u64 size)
{
  assert(size % 8 == 0);
  u64 const * encoded_blocks = (u64 *)input;
  u64 * decoded_blocks = (u64 *)output;
  u64 blocks_size = size / 8;

  u64 additional_key = VECTOR_KEY;
  
  for(u64 blocks_it = 0; blocks_it < blocks_size; ++blocks_it)
  {
    additional_key = encode_block(additional_key);
    u64 encoded_block = encoded_blocks[blocks_it];

    u64 decoded_block = additional_key ^ encoded_block;
    decoded_blocks[blocks_it] = decoded_block;
  }
}

static
void cbc_encode(u8 const * input, u8 * output, u64 size) // cipher block chaining
{
  assert(size % 8 == 0);
  u64 const * blocks = (u64 *)input;
  u64 * encoded_blocks = (u64 *)output;
  u64 blocks_size = size / 8;

  u64 first_block = blocks[0];
  first_block ^= VECTOR_KEY;
  
  u64 encoded_block = encode_block(first_block);
  encoded_blocks[0] = encoded_block;

  for(u64 blocks_it = 1; blocks_it < blocks_size; ++blocks_it)
  {
    u64 block = blocks[blocks_it];
    block ^= encoded_block;

    encoded_block = encode_block(block);
    encoded_blocks[blocks_it] = encoded_block;
  }
}

static
void cbc_decode(u8 const * input, u8 * output, u64 size)
{
  assert(size % 8 == 0);
  u64 const * encoded_blocks = (u64 *)input;
  u64 * decoded_blocks = (u64 *)output;
  u64 blocks_size = size / 8;

  u64 previous_block = encoded_blocks[0];
  
  u64 decoded_block = decode_block(previous_block);
  decoded_block ^= VECTOR_KEY;
  decoded_blocks[0] = decoded_block;

  for(u64 blocks_it = 1; blocks_it < blocks_size; ++blocks_it)
  {
    u64 block = encoded_blocks[blocks_it];

    decoded_block = decode_block(block);
    decoded_block ^= previous_block;
    decoded_blocks[blocks_it] = decoded_block;

    previous_block = block;
  }
}

static
void ecb_encode(u8 const * input, u8 * output, u64 size) // electronic codebook
{
  assert(size % 8 == 0);
  u64 const * blocks = (u64 *)input;
  u64 * encoded_blocks = (u64 *)output;
  u64 blocks_size = size / 8;

  for(u64 blocks_it = 0; blocks_it < blocks_size; ++blocks_it)
  {
    u64 block = blocks[blocks_it];

    u64 encoded_block = encode_block(block);
    encoded_blocks[blocks_it] = encoded_block;
  }
}

static
void ecb_decode(u8 const * input, u8 * output, u64 size)
{
  assert(size % 8 == 0);
  u64 const * encoded_blocks = (u64 *)input;
  u64 * decoded_blocks = (u64 *)output;
  u64 blocks_size = size / 8;

  for(u64 blocks_it = 0; blocks_it < blocks_size; ++blocks_it)
  {
    u64 block = encoded_blocks[blocks_it];

    u64 decoded_block = decode_block(block);
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

  ecb_encode(data, encoded_data, data_size);
  os_print("\n\nECB ENCODED:\n\n");

  for(u32 it = 0; it < data_size / 8; ++it)
  {
    u64 value = ((u64 *)encoded_data)[it];
    print("%u ", value);
  }
  
  ecb_decode(encoded_data, decoded_data, data_size);
  os_print("\n\nECB DECODED:\n\n");
  os_print_buffer(decoded_data, data_size);

  cbc_encode(data, encoded_data, data_size);
  os_print("\n\nCBC ENCODED:\n\n");

  for(u32 it = 0; it < data_size / 8; ++it)
  {
    u64 value = ((u64 *)encoded_data)[it];
    print("%u ", value);
  }
  
  cbc_decode(encoded_data, decoded_data, data_size);
  os_print("\n\nCBC DECODED:\n\n");
  os_print_buffer(decoded_data, data_size);

  ofb_encode(data, encoded_data, data_size);
  os_print("\n\nOFB ENCODED:\n\n");

  for(u32 it = 0; it < data_size / 8; ++it)
  {
    u64 value = ((u64 *)encoded_data)[it];
    print("%u ", value);
  }
  
  ofb_decode(encoded_data, decoded_data, data_size);
  os_print("\n\nOFB DECODED:\n\n");
  os_print_buffer(decoded_data, data_size);

  
  return(0);
}
