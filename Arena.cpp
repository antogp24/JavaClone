#include "Arena.h"
#include <stdlib.h>
#include <assert.h>

#if defined(_DEBUG) && (defined(_WIN32) || defined(_WIN64))
	#include <stdlib.h>
	#include <crtdbg.h>
	#define DBG_new new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
	#define DBG_new new
#endif

Arena arena_make() {
	Arena arena = {};
	arena_init(&arena, ARENA_DEFAULT_BUFFER_SIZE);
	return arena;
}

void arena_init(Arena* arena, size_t size) {
	arena->buffer = (uint8_t*)malloc(size);
	assert(arena->buffer != NULL);
	arena->length = size;
	arena->prev_offset = 0;
	arena->curr_offset = 0;
}

void arena_free(Arena *arena) {
	free(arena->buffer);
	arena->buffer = NULL;
	arena->length = 0;
	arena->prev_offset = 0;
	arena->curr_offset = 0;
}

void* arena_alloc_align(Arena *arena, size_t size, size_t align) {
	uintptr_t curr_ptr = (uintptr_t)arena->buffer + (uintptr_t)arena->curr_offset;
	uintptr_t offset = align_forward(curr_ptr, align);
	offset -= (uintptr_t)arena->buffer;

	if (offset + size > arena->length) {
		do {
			arena->length *= 2;
		} while (offset + size > arena->length);

		uint8_t *new_buffer = (uint8_t*)realloc(arena->buffer, arena->length);
		assert(new_buffer != NULL);
		arena->buffer = new_buffer;
	}

	void* ptr = &arena->buffer[offset];

	arena->prev_offset = offset;
	arena->curr_offset = offset + size;

	memset(ptr, 0, size);
	return ptr;
}

void* arena_alloc(Arena* arena, size_t size) {
	return arena_alloc_align(arena, size, ARENA_DEFAULT_ALIGNMENT);
}

inline bool is_power_of_two(uintptr_t x) {
	return (x & (x - 1)) == 0;
}

uintptr_t align_forward(uintptr_t ptr, size_t align) {
	uintptr_t p, a, modulo;
	assert(is_power_of_two(align));

	p = ptr;
	a = (uintptr_t)align;
	modulo = p & (a - 1);

	if (modulo != 0) {
		p += a - modulo;
	}
	return p;
}
