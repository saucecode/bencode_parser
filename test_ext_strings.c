/*
	Copyright (c) 2021 Julian Cahill <cahill.julian@gmail.com>

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	   claim that you wrote the original software. If you use this software
	   in a product, an acknowledgment in the product documentation would be
	   appreciated but is not required.
	2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.
	3. This notice may not be removed or altered from any source distribution.
*/

// https://github.com/codeplea/minctest
#include "minctest.h"

#define BENCODE_IMPLEMENTATION
// #define BENCODE_PRINT_ADDRESSES
#define BENCODE_EXT_STRINGS
#include "bencode.h"

#define bencode_parse_test_returns_end_of_str(string, bencode) \
	(bencode_parses((string), (bencode)) == (string) + strlen(string))

void test_string_basic() {
	struct bencode b = {0};
	
	char *c1 = "s\"hello, world!\"";
	
	lok(bencode_parses(c1, &b) == c1 + strlen(c1));
	lok(b.type == BENCODE_BYTES);
	assert(b.length > 0);
	lok(b.bytes[b.length - 1] == '\0');
	lequal(b.length, strlen("hello, world!") + 1);
	lok( memcmp(b.bytes, "hello, world!", b.length) == 0 );
	print_bencode(&b, 0);
	
}

void test_string_escape_sequence() {
	struct bencode b = {0};
	
	char *c1 = "s\"\\\"hello\\\\ world!\\\"\"";
	
	lok(bencode_parses(c1, &b) == c1 + strlen(c1));
	lok(b.type == BENCODE_BYTES);
	assert(b.length > 0);
	lok(b.bytes[b.length - 1] == '\0');
	lequal(b.length, strlen("\"hello\\ world!\"") + 1);
	lok( memcmp(b.bytes, "\"hello\\ world!\"", b.length) == 0 );
	print_bencode(&b, 0);
	
}

int main() {
	
	lrun("(extension) string parsing", test_string_basic);
	lrun("escape sequences", test_string_escape_sequence);
	
	lresults();
	
	return 0;
}