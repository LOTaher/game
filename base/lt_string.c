#include "lt.h"
#include <string.h>

String8 lt_string_substring(String8 str, u64 start, u64 end)
{
    end = MIN(end, str.length);
    start = MIN(start, end);

    return (String8){str.str + start, end - start};
}

String8 lt_string_cstring(char *str)
{
    return (String8){(u8 *)str, strlen(str)};
}

String8 lt_string_copy(String8 str, Arena *arena)
{
    u8 *bytes = (u8 *)lt_arena_push(arena, str.length);
    if (bytes == NULL)
    {
        return (String8){0};
    }

    memcpy(bytes, str.str, str.length);

    return (String8){bytes, str.length};
}

String8 lt_string_concat(String8 str1, String8 str2, Arena *arena)
{
    u8 *bytes = (u8 *)lt_arena_push(arena, str1.length + str2.length);
    if (bytes == NULL)
    {
        return (String8){0};
    }

    memcpy(bytes, str1.str, str1.length);
    memcpy(bytes + str1.length, str2.str, str2.length);

    return (String8){bytes, str1.length + str2.length};
}

b8 lt_string_compare(String8 str1, String8 str2)
{
    if (str1.length != str2.length) return 0;

    return memcmp(str1.str, str2.str, str1.length) == 0 ? 1 : 0;
}

b8 lt_string_contains(String8 str, String8 substr)
{
  if (substr.length > str.length)
    return 0;

  for (u64 i = 0; i < str.length; i++) {
    String8 sub = lt_string_substring(str, i, substr.length + i);

    if (lt_string_compare(sub, substr))
      return 1;
  }

  return 0;
}

