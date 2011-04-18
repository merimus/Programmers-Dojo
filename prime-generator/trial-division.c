
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/resource.h>
#include <sys/time.h>

#define THOUSAND 1000
#define MILLION 1000000

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
      prime_list->tail = elem;
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
  while( elem->prime <= s )
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
  struct timeval start;
  struct rusage ru;
  struct timeval tv;
  struct timeval elapsed;

  gettimeofday(&start, NULL);

  prime_list = calloc(1, sizeof(prime_list_t));
  push_prime(prime_list, 2);
  push_prime(prime_list, 3);

  uint64_t num = 4;
  while(1)
    {
      if( test_prime(prime_list, num) )
	{
	  push_prime(prime_list, num);

	  if( prime_list->length % (100*THOUSAND) == 0 )
	    {
	      getrusage(RUSAGE_SELF, &ru);
	      gettimeofday(&tv, NULL);
	      timersub(&tv, &start, &elapsed);
	      printf("num_primes: %.2f M Primes : %.2fMB : %.2f\n", prime_list->length / (double)MILLION, ru.ru_maxrss / 1024.0, elapsed.tv_sec+elapsed.tv_usec/(double)MILLION);
	    }
	}
      num++;
    }

  return 0;
}
