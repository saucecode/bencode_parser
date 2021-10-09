# bencode parser

A [bencode](https://en.wikipedia.org/wiki/Bencode) parser written in C. This code is not yet finished, and is not compliant with the standard! Malicious inputs may cause harm! Choking hazard. Avoid inhalation and contact with the eyes.

## Usage

This is a single file library. Define `BENCODE_IMPLEMENTATION` before including, in any one of your source files:

	#define BENCODE_IMPLEMENTATION
	#include "bencode.h"

Internally, integers are represented by `int64_t`, bytes are stored as `char*` with a `size_t length`. Lists and dictionaries (maps) are stored as linked lists. Read the header file for more information.

### Example usage:

	struct bencode b = {0};
	bencode_parse("ld4:name11:Pat Bateman6:heighti183eed4:name15:Evelyn Williams6:heighti157eee", &b);

	assert(b.type == BENCODE_LIST);
	
	struct bencode *head = b.list;
	while(head) {
		assert(head->type == BENCODE_DICT);
		
		struct bencode *name, *height;
		name = bencode_gets(head, "name");
		height = bencode_gets(head, "height");
		
		printf("%*s is %licm tall.\n", name->length, name->bytes, height->i);
		
		head = head->next;
	}