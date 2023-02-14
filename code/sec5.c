
#include "basic.c"

#define WIN_CONSOLE
#include "win.c"

#include "string.h"

static
u64 count_frequencies(u8 * text, u32 text_size, u32 * char_freq)
{
  u64 total_characters = 0;
  
  for(u32 it = 0; it < text_size; it += 1)
  {
    if((text[it] & 0xE0) == 0xC0)
    {
      u16 character = *((u16 *)(text + it));

      ++total_characters;
      ++char_freq[character];
      ++it;  
    }
  }
  print("total: %u\n", total_characters);

  return(total_characters);
}

static
void print_char_freq(u32 * occurs, u16 const * alphabet, u32 alphabet_size, u32 total_characters)
{
  HANDLE std_out = GetStdHandle(STD_OUTPUT_HANDLE);

  for(u32 it = 0; it < alphabet_size; ++it)
  {
    u16 character = alphabet[it];

    DWORD written;
    WriteConsole(std_out, &character, 2, &written, 0);  
    
    u32 occur = occurs[character];
    print(": %f\n", (f32)occur/(f32)total_characters);
  }
}

static
void decode_text(u8 * text, u32 text_size, u16 * decode_info)
{ 
  for(u32 it = 0; it < text_size; it += 1)
  {
    if((text[it] & 0xE0) == 0xC0)
    {
      u16 character = *((u16 *)(text + it));

      *((u16 *)(text + it)) = decode_info[character];
      
      ++it;  
    }
  } 
}

s32 main(void)
{
  u32 memory_size = mebibytes(10);
  memory_arena storage =
  {
    .base = os_allocate_memory(memory_size),
    .size = memory_size,
  };

  char const * alphabet = "абвгдеёжзийклмнопрстуфхцчшщъыьэюя";
  u16 const * two_byte_alphabet = (u16 const *)alphabet;
  u32 alphabet_size = 33;
  buffer text = read_entire_file_to_arena(&storage, "../data/sec5.data");
  
  u32 char_freq_size = 4 * 100000;
  u32 * char_freq = push_size(&storage, char_freq_size, 4);
  zero_out_memory(char_freq, char_freq_size);
  
  u32 total_characters = count_frequencies(text.memory, text.size, char_freq);

  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  print_char_freq(char_freq, two_byte_alphabet, alphabet_size, total_characters);

  u32 char_translations_size = 4 * 100000;
  u16 * char_translations = push_size(&storage, char_translations_size, 4);

  for(u32 it = 0; it < alphabet_size; ++it)
  {
    u16 character = two_byte_alphabet[it];
    char_translations[character] = character;
  }

  // tuning
  char_translations[two_byte_alphabet[19]] = two_byte_alphabet[15]; // т -> о
  char_translations[two_byte_alphabet[30]] = two_byte_alphabet[6]; // т -> о
  
  
  decode_text(text.memory, text.size, char_translations);

  HANDLE std_out = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD written;
  WriteConsole(std_out, text.memory, text.size, &written, 0);  

  
  return(0);
}
