#include "Alphabet.hpp"

constexpr int BASE_ALPHABET_SIZE = 150;

const int ALPHABET_SIZE = BASE_ALPHABET_SIZE + (int)RESERVED_SYMBOLS.size();

char get_position(char c) { return (c) + (char)RESERVED_SYMBOLS.size(); }