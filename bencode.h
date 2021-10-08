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
		BENCODE_INT, BENCODE_BYTES,
		BENCODE_LIST, BENCODE_DICT
	} type;
	union {
		int64_t i;
		
		char *bytes;
		
		struct bencode *list;
	};
	
	size_t length; // BENCODE_BYTES
	
	struct bencode *next; // BENCODE_DICT | BENCODE_LIST
	char *key; // BENCODE_DICT
};

char* bencode_parse(char *str, struct bencode *dest);
void print_bencode(struct bencode *b, int indent);
void bencode_free(struct bencode *b);

#ifdef BENCODE_IMPLEMENTATION

char* bencode_parse(char *str, struct bencode *dest) {
	if(BENCODE_DEBUG_PRINTS) printf("PARSING: %s\n", str);
	size_t length = strlen(str);
	
	if(length < 2) return str;
	
	dest->next = NULL;
	
	if(str[0] == 'i') {
		char *e = strchr(str, 'e');
		char integer_read[24] = {0};
		
		if( (e - (str+1)) > (long int) sizeof(integer_read) ) return str;
		
		strncpy(integer_read, str+1, e - str);
		
		dest->type = BENCODE_INT;
		dest->i = atol(integer_read);
		if(BENCODE_DEBUG_PRINTS) printf("load int %li\n", dest->i);
		
		return e + 1;
		
	} else if(str[0] >= '0' && str[0] <= '9') {
		char *colon = strchr(str, ':');
		char integer_read[24] = {0};
		
		if( colon - str > (long int) sizeof(integer_read) ) return str;
		
		strncpy(integer_read, str, colon - str);
		size_t bytes_length = atol(integer_read);
		
		dest->type = BENCODE_BYTES;
		dest->length = bytes_length;
		dest->bytes = malloc(bytes_length);
		memcpy(dest->bytes, colon+1, bytes_length);
		if(BENCODE_DEBUG_PRINTS) printf("load bytes (advance cursor by %li) ", colon + bytes_length + 1 - str);
		if(BENCODE_DEBUG_PRINTS) for(unsigned int i=0; i<dest->length;i++) printf("%c", dest->bytes[i]);
		if(BENCODE_DEBUG_PRINTS) printf("\n");
		
		return colon + bytes_length + 1;
		
	} else if(str[0] == 'l') {
		dest->type = BENCODE_LIST;
		dest->list = NULL;
		if(BENCODE_DEBUG_PRINTS) printf("start list %p\n", str);
		
		int first = 1;
		
		struct bencode *head;
		
		str++;
		
		while(str[0] != 'e') {
			if(first) {
				dest->list = malloc(sizeof(struct bencode));
				memset(dest->list, 0, sizeof(struct bencode));
				head = dest->list;
				
				first = 0;
				
			} else {
				head->next = malloc(sizeof(struct bencode));
				memset(head->next, 0, sizeof(struct bencode));
				head = head->next;
			
			}
			
			char *next = bencode_parse(str, head);
			
			assert(str != next);
			
			if(BENCODE_DEBUG_PRINTS) printf("list jumped forward %li bytes\n", next - str);
			str = next;
		}
		
		if(BENCODE_DEBUG_PRINTS) printf("end list %p\n", str);
		return str+1;
		
	}
	
	return str;
}

void print_bencode(struct bencode *b, int indent) {
	for(int i = 0; i < indent; i++)
		printf("\t");
		
	printf("struct bencode {addr=%p, next=%p, type=%u, ", (void*) b, (void*) b->next, b->type);
	
	if(b->type == BENCODE_INT)
		printf("int=%li", b->i);
	
	if(b->type == BENCODE_BYTES) {
		printf("length=%zu, bytes=\"", b->length);
		for(unsigned int i = 0; i < b->length; i++) printf("%c", b->bytes[i]);
		printf("\"");
	}
	
	if(b->type == BENCODE_LIST) {
		printf("list=%p\n", (void*) b->list);
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
	}
	
	if(b->type == BENCODE_LIST) {
		struct bencode *head = b->list;
		
		while(head) {
			struct bencode *prev = head;
			head = head->next;
			
			bencode_free(prev);
			free(prev);
		}
	}
}

#endif // BENCODE_IMPLEMENTATION

#endif // HEADER GUARD