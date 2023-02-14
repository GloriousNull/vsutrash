
#include "basic.c"

#define WIN_CONSOLE
#include "win.c"

static
f32 next_random(f32 number)
{
  f32 M = 13.0f; // 13 is the best value so far
  number *= M;
  
  f32 result = number - (s32)number;
  return(result);
}

static
void generate_samples(f32 * samples, u64 samples_size, f32 random_value)
{
  samples[0] = random_value;
  for(u32 it = 1; it < samples_size; ++it)
  {
    random_value = next_random(random_value);
    samples[it] = random_value;
  }
}

static
void nonuniformity_test(f32 * samples, u64 samples_size, u32 * hist, u32 hist_size)
{
  f32 hist_step = 1.0f / hist_size;

  u32 hist_index = 0;
  f32 low_border = 0.0f;
  f32 high_border = hist_step;

  for(u32 hist_it = 0; hist_it < hist_size; ++hist_it)
  {
    for(u64 it = 0; it < samples_size; ++it)
    {
      if(samples[it] <= high_border && samples[it] >= low_border)
      {
	++hist[hist_index];
      }
    }

    ++hist_index;
    high_border += hist_step;
    low_border += hist_step;
  }

  f32 average_hist_hit_ratio = (f32)samples_size / (f32)hist_size;
  f32 sum = 0.0f;
  for(u32 it = 0; it < hist_size; ++it)
  {
    sum += (f32)(hist[it]);
  }

  sum -= average_hist_hit_ratio;
  sum /= hist_size;

  sum = square_root(sum);
  
  f32 coeff = ((1.0f / average_hist_hit_ratio) * sum * 100.0f);
  print("nonuniformity: %f%% on %u samples by %u hist\n", coeff, samples_size, hist_size);
}

static
u64 length_test(f32 * samples, u64 samples_size, f32 random_value)
{
  samples[0] = random_value;

  u64 repeats = 0;
  u64 first_repeat = samples_size;
  
  u64 min_distance_pos_0 = 0;
  u64 min_distance_pos_1 = 0;
  u64 min_distance = ~0ull;
  
  u64 max_distance_pos_0 = 0;
  u64 max_distance_pos_1 = 0;
  u64 max_distance = 0;
  for(u64 it_0 = 1; it_0 < samples_size; ++it_0)
  {
    random_value = next_random(random_value);
    samples[it_0] = random_value;

    for(u64 it_1 = 0; it_1 < it_0; ++it_1)
    {
      if(random_value == samples[it_1])
      {
	if(first_repeat == samples_size)
	  first_repeat = it_0 + 1;
	++repeats;
	u64 distance = it_0 - it_1;
	if(distance > max_distance)
	{
	  max_distance = distance;
	  max_distance_pos_0 = it_0;
	  max_distance_pos_1 = it_1;
	}

	if(distance < min_distance)
	{
	  min_distance = distance;
	  min_distance_pos_0 = it_0;
	  min_distance_pos_1 = it_1;
	}
	
	print("found repeat: %f, pos0: %u, pos1: %u, distance: %u\n", random_value, it_1, it_0, distance);
      }
    }
  }

  print("min distance: %u, pos0: %u, pos1: %u\n", min_distance, min_distance_pos_0, min_distance_pos_1);
  print("max distance: %u, pos0: %u, pos1: %u\nsize: %u\nrepeats: %u\n", max_distance, max_distance_pos_0, max_distance_pos_1, samples_size, repeats);

  return first_repeat;
}

s32 main(void)
{
  u64 memory_size = mebibytes(10);
  memory_arena storage =
  {
    .base = os_allocate_memory(memory_size),
    .size = memory_size,
  };

  f32 u = 2.0f; // 27, 2
  f32 p = 11765.0f;
  f32 seed = u / p;
  
  u64 samples_size = 250000;
  f32 * samples = push_size(&storage, samples_size, 4);
  zero_out_memory(samples, samples_size * 4);

  u64 valid_range_size = length_test(samples, samples_size, seed);

  u32 hist_size = 10;
  u32 * hist = push_size(&storage, hist_size, 4);
  zero_out_memory(hist, hist_size * 4);
  nonuniformity_test(samples, valid_range_size, hist, hist_size);
  
  print("seed: %f\n", seed);
  
  return(0);
}
