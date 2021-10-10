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

#ifndef BENCODE_H
#define BENCODE_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "assert.h"

#ifndef BENCODE_DEBUG_PRINTS
#define BENCODE_DEBUG_PRINTS 0
#endif

struct bencode {
	enum {
		BENCODE_INT = 1, BENCODE_BYTES,
		BENCODE_LIST, BENCODE_DICT
	} type;
	
	union {
		int64_t i;            // BENCODE_INT
		char *bytes;          // BENCODE_BYTES
		struct bencode *list; // BENCODE_LIST
		struct bencode *dict; // BENCODE_DICT
	};
	
	char *key; // BENCODE_DICT
	
	union {
		size_t length;     // BENCODE_BYTES
		size_t key_length; // BENCODE_DICT
	};
	
	struct bencode *next; // BENCODE_DICT | BENCODE_LIST
};

char* bencode_parse(char *str, struct bencode *dest);
void print_bencode(struct bencode *b, int indent);
void bencode_free(struct bencode *b);
struct bencode* bencode_gets(struct bencode *b, char *key_string); 

#ifdef BENCODE_IMPLEMENTATION

#include "ctype.h"

struct bencode* bencode_gets(struct bencode *b, char *key_string) {
	if(b->type != BENCODE_DICT) return NULL;
	
	size_t query_length = strlen(key_string);
	
	struct bencode *head = b->dict;
	while(head) {
		if( 0 == memcmp(
		           head->key,
		           key_string,
		           (head->key_length > query_length) ? query_length : head->key_length
			)
		) return head;
		head = head->next;
	}
	
	return head;
}

char* bencode_parse(char *str, struct bencode *dest) {
	if(BENCODE_DEBUG_PRINTS) printf("PARSING: %s\n", str);
	size_t length = strlen(str);
	
	if(length < 2) return str;
	
	dest->next = NULL;
	
	if(str[0] == 'i') {
		char *e = strchr(str, 'e');
		char integer_read[24] = {0};
		
		if(e == str+1) return str; // no blank inputs
		if( (e - (str+1)) > (long int) sizeof(integer_read) ) return str;
		if( memcmp(str+1, "-0", 2) == 0 ) return str; // no negative leading zeros
		// if( (e - str) < 2 && str[1] == '0' ) return str; // no leading zeros
		// if(isspace(str[1])) return str; // no leading whitespace
		
		int enumerate = 0;
		for(char *cursor = str + 1; cursor < e; cursor++, enumerate++) {
			if(*cursor == '-' && enumerate == 0) continue;
			
			if( !(*cursor >= '0' && *cursor <= '9') ) return str;
		}
		
		strncpy(integer_read, str+1, e - str);
		
		// dest->i = atol(integer_read);
		if( sscanf(str, "i%lie", &dest->i) != 1 ) return str;
		if(BENCODE_DEBUG_PRINTS) printf("load int %li\n", dest->i);
		
		dest->type = BENCODE_INT;
		
		return e + 1;
		
	} else if(str[0] >= '0' && str[0] <= '9') {
		char *colon = strchr(str, ':');
		char integer_read[24] = {0};
		
		if(colon == NULL) return str;
		
		if( colon - str > (long int) sizeof(integer_read) ) return str;
		
		strncpy(integer_read, str, colon - str);
		size_t bytes_length = atol(integer_read);
		
		if( (size_t) (colon + bytes_length) >= (size_t) (str + length) ) return str;
		
		dest->type = BENCODE_BYTES;
		dest->length = bytes_length;
		dest->bytes = malloc(bytes_length);
		memcpy(dest->bytes, colon+1, bytes_length);
		if(BENCODE_DEBUG_PRINTS) printf("load bytes (advance cursor by %li) ", colon + bytes_length + 1 - str);
		if(BENCODE_DEBUG_PRINTS) for(unsigned int i=0; i<dest->length;i++) printf("%c", dest->bytes[i]);
		if(BENCODE_DEBUG_PRINTS) printf("\n");
		
		return colon + bytes_length + 1;
		
	} else if(str[0] == 'l' || str[0] == 'd') {
		dest->type = (str[0] == 'l') ? BENCODE_LIST : BENCODE_DICT;
		dest->list = NULL;
		dest->dict = NULL;
		if(BENCODE_DEBUG_PRINTS && dest->type == BENCODE_LIST) printf("start list %p\n", str);
		if(BENCODE_DEBUG_PRINTS && dest->type == BENCODE_DICT) printf("start dict %p\n", str);
		
		int first = 1;
		
		struct bencode *head;
		
		char *original_str = str;
		str++;
		
		while(str[0] != 'e' && str < original_str + length) {
			if(first) {
				struct bencode **pog = (dest->type == BENCODE_LIST) ? &dest->list : &dest->dict;
				*pog = malloc(sizeof(struct bencode));
				memset(*pog, 0, sizeof(struct bencode));
				head = *pog;
				
				first = 0;
				
			} else {
				head->next = malloc(sizeof(struct bencode));
				memset(head->next, 0, sizeof(struct bencode));
				head = head->next;
			
			}
			
			if(dest->type == BENCODE_LIST) {
				char *next = bencode_parse(str, head);
				
				// assert(str != next);
				if(str == next) return str;
				
				if(BENCODE_DEBUG_PRINTS) printf("list jumped forward %li bytes\n", next - str);
				str = next;
				
			} else if(dest->type == BENCODE_DICT) {
				// load the key into a temporary struct
				struct bencode key = {0};
				char *next = bencode_parse(str, &key);
				
				// verify the key is bytes type
				if(key.type != BENCODE_BYTES) return str;
				assert(str != next);
				
				str = next;
				
				// load the key into the head
				head->key = key.bytes;
				head->key_length = key.length;
				
				// load the value into the head
				next = bencode_parse(str, head);
				
				assert(str != next);
				str = next;
				
			}
		}
		
		if(BENCODE_DEBUG_PRINTS) printf("end list %p\n", str);
		return str+1;
		
	}
	
	return str;
}

void print_bencode(struct bencode *b, int indent) {
	for(int i = 0; i < indent; i++)
		printf("\t");
		
	if(!b->type) {
		printf("struct bencode {addr=%p, type=(uninitialized)}\n", (void*) b);
		return;
	}

	printf("struct bencode {addr=%p, next=%p, type=%u, ", (void*) b, (void*) b->next, b->type);
	
	if(b->key) {
		printf("key=\"");
		for(unsigned int i = 0; i < b->key_length; i++) printf("%c", b->key[i]);
		printf("\", ");
	}
	
	if(b->type == BENCODE_INT)
		printf("int=%li", b->i);
	
	if(b->type == BENCODE_BYTES) {
		printf("length=%zu, bytes=\"", b->length);
		for(unsigned int i = 0; i < b->length; i++) printf("%c", b->bytes[i]);
		printf("\"");
	}
	
	if(b->type == BENCODE_LIST || b->type == BENCODE_DICT) {
		printf( ((b->type == BENCODE_LIST) ? "list=%p\n" : "dict=%p\n"), (void*) b->list);
		struct bencode *element = b->list;
		
		while(element) {
			print_bencode(element, indent+1);
			element = element->next;
		}
	}
	
	if(b->type == BENCODE_LIST || b->type == BENCODE_DICT)
		for(int i = 0; i < indent; i++)
			printf("\t");
	
	printf("}\n");
}

void bencode_free(struct bencode *b) {
	if(b->type == BENCODE_BYTES) {
		free(b->bytes);
		b->bytes = NULL;
	}
	
	if(b->type == BENCODE_LIST || b->type == BENCODE_DICT) {
		struct bencode *head = (b->type == BENCODE_LIST) ? b->list : b->dict;
		
		while(head) {
			struct bencode *prev = head;
			head = head->next;
			
			bencode_free(prev);
			if(b->type == BENCODE_DICT) {
				free(prev->key);
				prev->key = NULL;
			}
			free(prev);
		}
	}
}

#endif // BENCODE_IMPLEMENTATION

#endif // HEADER GUARD