#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
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

void sieve(uint64_t root, uint64_t n, char numbers[], prime_list_t *primes)
{
  for( int cnt = 0; cnt < n; cnt++ )
    {numbers[cnt] = 1;}

  prime_elem_t *elem = primes->head;
  while( elem )
    {
      uint64_t start;
      uint64_t step = elem->prime;
      
      if( root % step != 0 )
	{start = step - (root % step);}
      else
	{start = 0;}

      if( start >= n )
	return;

      while( start < n )
	{
	  numbers[start] = 0;
	  start += step;
	}
      elem = elem->next;
    }

    for( int cnt = 0; cnt < n; cnt++ )
    {
      if( numbers[cnt] != 0 )
	{
	  int step = root+cnt;
	  int start = cnt + step;

	  if( start >= n )
	    return;

	  while( start < n )
	    {
	      numbers[start] = 0;
	      start += step;
	    }
	}
    }
}

int main(int argc, char *argv[])
{
  int64_t size = atoi(argv[1])*MILLION;
  char *numbers;
  prime_list_t *prime_list;
  struct timeval start;
  struct rusage ru;
  struct timeval tv;
  struct timeval elapsed;

  uint64_t root = 2;

  gettimeofday(&start, NULL);

  numbers = malloc(size);

  prime_list = calloc(1, sizeof(prime_list_t));

  while( 1 )
    {
      sieve(root, size, numbers, prime_list);
      for( int cnt = 0; cnt < size; cnt++ )
	{
	  if( numbers[cnt] )
	    {
	      push_prime(prime_list, cnt+root);

	      if( prime_list->length % MILLION == 0 )
		{
		  getrusage(RUSAGE_SELF, &ru);
		  gettimeofday(&tv, NULL);
		  timersub(&tv, &start, &elapsed);
		  printf("num_primes: %.2f M Primes : %.2fMB : %.2f\n", prime_list->length / (double)MILLION, ru.ru_maxrss / 1024.0, elapsed.tv_sec+elapsed.tv_usec/(double)MILLION);
		}
	    }
	}
      root += size;
    }

  return 0;
}
