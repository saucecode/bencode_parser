// https://github.com/codeplea/minctest
#include "minctest.h"

#define BENCODE_IMPLEMENTATION
#include "bencode.h"

#define bencode_parse_test_returns_end_of_str(string, bencode) \
	(bencode_parse((string), (bencode)) == (string) + strlen(string))

void test_int() {
	struct bencode b;
	
	char *c1 = "i42e";
	char *c2 = "i-1000e";
	
	lok(bencode_parse(c1, &b) == c1 + strlen(c1));
	llequal(b.i, 42l);
	
	lok(bencode_parse(c2, &b) == c2 + strlen(c2));
	llequal(b.i, -1000l);
	
	// char *c3 = "i02e";
	// lok(bencode_parse(c3, &b) == c3);
	
}

void test_sequence() {
	struct bencode b, c;
	char *c1 = "i42e4:eggs";
	char *c2 = bencode_parse(c1, &b);
	lok(c2 != c1);
	char *c3 = bencode_parse(c2, &c);
	lok(c3 != c2);
	lok(c3 == c1 + strlen(c1));
	lok(c2[0] == '4');
	lok(c3[0] == '\0');
	
	bencode_free(&b);
	bencode_free(&c);
}

void test_bytes() {
	struct bencode b;
	
	char *c1 = "11:hello world";
	char *c2 = "0:";
	
	lok(bencode_parse_test_returns_end_of_str(c1, &b));
	lok(b.type == BENCODE_BYTES);
	lok(strlen("hello world") == b.length);
	lok( memcmp(b.bytes, "hello world", 11) == 0 );
	
	print_bencode(&b, 0);
	bencode_free(&b);
	
	lok(bencode_parse(c2, &b) == c2 + strlen(c2));
	lok(b.type == BENCODE_BYTES);
	lok(b.length == 0);
	
	bencode_free(&b);
}

void test_list_simple() {
	struct bencode b;
	
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
	
	print_bencode(&b, 0);
	bencode_free(&b);
}

void test_list_nested() {
	struct bencode b;
	
	// char *c1 = "l6:Julianli80ele1:3ee";
	char *c1 = "l6:Julianli18eeleli0ei1ei2eee";
	lok(bencode_parse_test_returns_end_of_str(c1, &b));
	print_bencode(&b, 0);
	lok(b.list->type == BENCODE_BYTES);
	lok(b.list->next->type == BENCODE_LIST);
	lok(b.list->next->next->type == BENCODE_LIST);
	lok(b.list->next->next->next->type == BENCODE_LIST);
	lok(b.list->next->next->next->next == NULL);
	bencode_free(&b);
	// lok(b.list->next->next->i != 18);
}

int main() {
	
	lrun("integer parsing", test_int);
	lrun("bytes parsing", test_bytes);
	lrun("sequencing", test_sequence);
	lrun("lists parsing", test_list_simple);
	lrun("lists (complex) parsing", test_list_nested);
	lresults();
	
	return 0;
}