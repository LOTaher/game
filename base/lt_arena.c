#include "lt.h"
#include <string.h>

Arena* lt_arena_create(u64 capacity)
{
    u64 page_size = lt_mem_page_size();
    capacity = lt_arena_align_forward(capacity, page_size);

    void* base = lt_mem_reserve(capacity);
    if (base == NULL) {
        return NULL;
    }

    u64 initial_commit = lt_arena_align_forward(sizeof(Arena) + LT_ARENA_COMMIT_CHUNK, page_size);
    if (initial_commit > capacity) {
        initial_commit = capacity;
    }

    if (!lt_mem_commit(base, initial_commit)) {
        lt_mem_release(base, capacity);
        return NULL;
    }

    Arena* arena = (Arena*)base;
    arena->capacity  = capacity;
    arena->committed = initial_commit;
    arena->pos       = sizeof(*arena);
    return arena;
}

void lt_arena_destroy(Arena* arena)
{
    lt_mem_release(arena, arena->capacity);
}

u64 lt_arena_align_forward(u64 pos, u64 alignment)
{
    return (pos + (alignment - 1)) & ~(alignment - 1);
}

void* lt_arena_push(Arena* arena, u64 size)
{
    u64 aligned_pos = lt_arena_align_forward(arena->pos, sizeof(void*));
    u64 new_pos = aligned_pos + size;

    if (new_pos > arena->capacity) {
        return NULL;
    }

    if (new_pos > arena->committed) {
        u64 page_size = lt_mem_page_size();
        u64 needed = lt_arena_align_forward(new_pos, page_size);
        u64 grow_to = MAX(needed, arena->committed + LT_ARENA_COMMIT_CHUNK);
        grow_to = MIN(grow_to, arena->capacity);

        u64 commit_size = grow_to - arena->committed;
        u8* commit_ptr = (u8*)arena + arena->committed;

        if (!lt_mem_commit(commit_ptr, commit_size)) {
            return NULL;
        }
        arena->committed = grow_to;
    }

    arena->pos = new_pos;
    u8* block = (u8*)arena + aligned_pos;
    memset(block, 0, size);
    return block;
}

void lt_arena_clear(Arena* arena)
{
    arena->pos = sizeof(*arena);
}

u64 lt_arena_mark(Arena* arena)
{
    return arena->pos;
}

void lt_arena_pop(Arena* arena, u64 mark)
{
    if (mark < sizeof(*arena)) {
        mark = sizeof(*arena);
    }
    arena->pos = mark;
}

Arena_Temp lt_arena_temp_begin(Arena* arena)
{
    Arena_Temp temp = {0};
    temp.arena = arena;
    temp.pos = arena->pos;
    return temp;
}

void lt_arena_temp_end(Arena_Temp arena)
{
    lt_arena_pop(arena.arena, arena.pos);
}

