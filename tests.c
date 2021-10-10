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
#include "bencode.h"

#define bencode_parse_test_returns_end_of_str(string, bencode) \
	(bencode_parses((string), (bencode)) == (string) + strlen(string))

void test_int() {
	struct bencode b = {0};
	
	char *c1 = "i42e";
	char *c2 = "i-1000e";
	
	lok(bencode_parses(c1, &b) == c1 + strlen(c1));
	llequal(b.i, 42l);
	
	lok(bencode_parses(c2, &b) == c2 + strlen(c2));
	llequal(b.i, -1000l);
	
	// char *c3 = "i02e";
	// lok(bencode_parses(c3, &b) == c3);
	
}

void test_invalid() {
	char *no_advance_tests[] = {
		// integers
		"ie",
		"iNi!e",
		"i-0e",
		"i000030"
		"i-e",
		"i 5e",
		"i2 2e",
		(char[]) {'i', 100, 90, 33, 1, 'e'},
		"i-2 e",
		"ei",
		"i5              ",
		
		// bytes
		"-1:abc", // invalid length
		"5:abcd", // buffer overflow
	};
	
	int tests_count = sizeof(no_advance_tests) / sizeof(char*);
	for(int i = 0; i < tests_count; i++) {
		printf("Testing string \"%s\"\n", no_advance_tests[i]);
		struct bencode b = {0};
		lok(bencode_parses(no_advance_tests[i], &b) == no_advance_tests[i]);
		lok(b.type == 0);
		// print_bencode(&b, 0);
		bencode_free(&b);
	}
}

void test_invalid_lists() {
	char *uninitialized_list_tests[] = {
		// lists
		"l5e",
		"lie",
		"l i42ee",
		"l i24e e",
		"ll"
	};
	
	int tests_count = sizeof(uninitialized_list_tests) / sizeof(char*);
	for(int i = 0; i < tests_count; i++) {
		char *str = uninitialized_list_tests[i];
		printf("Testing string \"%s\"\n", str);
		struct bencode b = {0};
		lok(bencode_parses(str, &b) == str + 1);
		lok(b.type == BENCODE_LIST);
		lok(b.list != NULL);
		lok(b.list->type == 0);
		print_bencode(&b, 0);
		bencode_free(&b);
	}
	
	char *single_element_list_tests[] = {
		"li24e",
		"llele"
	};
	
	tests_count = sizeof(single_element_list_tests) / sizeof(char*);
	for(int i = 0; i < tests_count; i++) {
		char *str = single_element_list_tests[i];
		printf("Testing string \"%s\"\n", str);
		struct bencode b = {0};
		lok(bencode_parses(str, &b) != str + strlen(str));
		lok(b.type == BENCODE_LIST);
		lok(b.list != NULL);
		lok(b.list->type != 0);
		print_bencode(&b, 0);
		bencode_free(&b);
	}
}

void test_sequence() {
	struct bencode b = {0}, c = {0};
	char *c1 = "i42e4:eggs";
	char *c2 = bencode_parses(c1, &b);
	lok(c2 != c1);
	char *c3 = bencode_parses(c2, &c);
	lok(c3 != c2);
	lok(c3 == c1 + strlen(c1));
	lok(c2[0] == '4');
	lok(c3[0] == '\0');
	
	bencode_free(&b);
	bencode_free(&c);
}

void test_bytes() {
	struct bencode b = {0};
	
	char *c1 = "11:hello world";
	char *c2 = "0:";
	
	lok(bencode_parse_test_returns_end_of_str(c1, &b));
	lok(b.type == BENCODE_BYTES);
	lok(strlen("hello world") == b.length);
	lok( memcmp(b.bytes, "hello world", 11) == 0 );
	
	// print_bencode(&b, 0);
	bencode_free(&b);
	
	lok(bencode_parses(c2, &b) == c2 + strlen(c2));
	lok(b.type == BENCODE_BYTES);
	lok(b.length == 0);
	
	bencode_free(&b);
}

void test_list_simple() {
	struct bencode b = {0};
	
	char *c1 = "li42e4:spame";
	lok(bencode_parse_test_returns_end_of_str(c1, &b));
	lok(b.type == BENCODE_LIST);
	lequal(b.list->type, BENCODE_INT);
	llequal(b.list->i, 42l);
	
	lok(b.list->next);
	lok(b.list->next->type == BENCODE_BYTES);
	lok(b.list->next->length == 4);
	lok( memcmp(b.list->next->bytes, "spam", b.list->next->length) == 0 );
	lok(b.list->next->next == NULL);
	
	// print_bencode(&b, 0);
	bencode_free(&b);
}

void test_list_nested() {
	struct bencode b = {0};
	
	// char *c1 = "l6:Julianli80ele1:3ee";
	char *c1 = "l6:Julianli18eeleli0ei1ei2eee";
	lok(bencode_parse_test_returns_end_of_str(c1, &b));
	// print_bencode(&b, 0);
	lok(b.list->type == BENCODE_BYTES);
	lok(b.list->next->type == BENCODE_LIST);
	lok(b.list->next->next->type == BENCODE_LIST);
	lok(b.list->next->next->next->type == BENCODE_LIST);
	lok(b.list->next->next->next->next == NULL);
	bencode_free(&b);
	// lok(b.list->next->next->i != 18);
}

void test_dict() {
	struct bencode b = {0};
	
	char *c1 = "d6:bobcat13:Melbourne, AUe";
	lok(bencode_parse_test_returns_end_of_str(c1, &b));
	// print_bencode(&b, 0);
	
	bencode_free(&b);
}

void test_dict_composite() {
	struct bencode b = {0};
	
	char *c1 = "d4:named5:first7:Winston4:last10:Churchhille3:agei69ee";
	lok(bencode_parse_test_returns_end_of_str(c1, &b));
	// print_bencode(&b, 0);
	
	bencode_free(&b);
}

void test_dict_retrieve() {
	struct bencode b = {0};
	
	char *c1 = "d4:named5:first7:Winston4:last10:Churchhille3:agei69ee";
	lok(bencode_parse_test_returns_end_of_str(c1, &b));
	// // print_bencode(&b, 0);
	
	struct bencode *age = bencode_gets(&b, "age");
	lok(age != NULL);
	lok(age->type == BENCODE_INT);
	lok(memcmp("age", age->key, 3) == 0);
	lok(age->i == 69);
	
	struct bencode *name, *lastname;
	if( (name = bencode_gets(&b, "name")) ) {
		if( (lastname = bencode_gets(name, "last")) ) {
			lok(lastname->type == BENCODE_BYTES);
			lok(memcmp("last", lastname->key, 4) == 0);
			lok(memcmp("Churchhill", lastname->bytes, 10) == 0);
		} else
			lok(0);
	} else
		lok(0);
	
	bencode_free(&b);
}

void test_dict_error() {
	struct bencode b = {0};
	
	char *c1 = "d5:quest5:grail5:color4:bluei16e3:Ni!e";
	
	char *c2 = bencode_parses(c1, &b);
	lok( c2 != c1 + strlen(c1) );
	// print_bencode(&b, 0);
	printf("Bencode parsing stopped at %s\n", c2);
	
	bencode_free(&b);
}

int main() {
	
	lrun("integer parsing", test_int);
	lrun("bytes parsing", test_bytes);
	lrun("sequencing", test_sequence);
	lrun("lists parsing", test_list_simple);
	lrun("lists (complex) parsing", test_list_nested);
	lrun("dictionary parsing", test_dict);
	lrun("dictionary composite parsing", test_dict_composite);
	lrun("dictionary parsing (with invalid inputs)", test_dict_error);
	lrun("dictionary get functions", test_dict_retrieve);
	
	lrun("invalid inputs (bytes and ints)", test_invalid);
	lrun("invalid inputs (lists)", test_invalid_lists);
	lresults();
	
	return 0;
}