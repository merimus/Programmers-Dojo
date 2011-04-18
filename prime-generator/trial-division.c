
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct prime_elem_s {
  uint64_t prime;
  struct prime_elem_s *next;
};
typedef struct prime_elem_s prime_elem_t;

typedef struct {
  uint64_t length;
  prime_elem_t *head;
  prime_elem_t *tail;
} prime_list_t;

void push_prime(prime_list_t *prime_list, uint64_t prime)
{
  prime_elem_t *elem = malloc(sizeof(prime_elem_t));
  elem->prime = prime;
  elem->next = NULL;

  if( prime_list->tail )
    {
      prime_list->tail->next = elem;
    } 
  else 
    {
      prime_list->head = prime_list->tail = elem;
    }
  prime_list->length++;
}

int test_prime(prime_list_t *prime_list, uint64_t num)
{
  prime_elem_t *elem = prime_list->head;
  uint64_t s = sqrt(num);
  while( elem && elem->prime <= s )
    {
      if( num % elem->prime == 0 )
	{
	  return 0;
	}
      elem = elem->next;
    }
  return 1;
}

int main(int argc, char *argv[])
{
  prime_list_t *prime_list;
  prime_list = calloc(1, sizeof(prime_list_t));
  push_prime(prime_list, 2);

  uint64_t n = 3;
  while(1)
    {
      if( test_prime(prime_list, n) )
	{
	  push_prime(prime_list, n);
	  if( prime_list->length % (10*1000*1000) == 0 )
	    printf("num_primes: %llu\n", prime_list->length);
	  if( prime_list->length == 100*1000*1000 )
	    return 1;
	}
      n++;
    }

  return 0;
}
