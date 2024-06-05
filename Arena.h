#pragma once

// Basic arena implementation. Heavily inspired by:
// https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/

#include <memory.h>
#include <stdint.h>

#define ARENA_DEFAULT_BUFFER_SIZE 1024
#define ARENA_DEFAULT_ALIGNMENT (2 * sizeof(void*))

struct Arena {
	uint8_t *buffer;
	size_t length;
	size_t curr_offset;
	size_t prev_offset;
};

inline bool is_power_of_two(uintptr_t x);
uintptr_t align_forward(uintptr_t ptr, size_t align);

Arena arena_make();
void arena_init(Arena *arena, size_t size);
void arena_free(Arena *arena);
void* arena_alloc_align(Arena* arena, size_t size, size_t align);
void* arena_alloc(Arena* arena, size_t size);

#define arena_push_type(arena, T) (T*)arena_alloc(arena, sizeof(T))

