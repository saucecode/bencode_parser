# bencode parser

A [bencode](https://en.wikipedia.org/wiki/Bencode) parser written in C. This code is not yet finished, and malicious inputs may cause harm! Choking hazard. Avoid inhalation and contact with the eyes.

This library will also implement some optional extensions, which can be enabled with preprocessor definitions. See the extensions section below.

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

## Extensions

These are non-standard, and invented by myself. Insert these defines alongside your `#define BENCODE_IMPLEMENTATION` line to enable their implementations.

### `#define BENCODE_EXT_WHITESPACE`

Permits whitespace between elements of dicts and lists. Whitespace includes the usual suspects, as described by C's `isspace(char)` function. This was added to improve readability, making bencode easier to write by hand. An example valid under this extension follows.

`l i32e i33e     d 17:airspeed_velocity i11e 6:laden? 5:False e 17:I seek the grail!e`

### `#define BENCODE_EXT_WHITESPACE` (work in progress)

Allows a new syntax for defining byte arrays without prefixing the length. Example inputs and their equivalent standard-syntax inputs are shown in the table below.

| extension syntax                 | standard syntax                | literal output
| -------------------------------- | ------------------------------ | ---------------
| `s"Hello, world!"`               | `13:Hello, world!`             | `Hello, world!`
| `s"Who is this \"4-chan\"?"`     | `21:Who is this "4-chan"?`     | `Who is this "4-chan"?`
| `s"Backslash \"\\\" in quotes!"` | `24:Backslash "\" in quotes!`  | `Backslash "\" in quotes!`

## License

zlib