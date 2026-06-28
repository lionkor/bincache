#include "arena.hpp"
void detail::ArenaDtor::operator()(Arena* arena) {
    arena_destroy(arena);
}
