#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>

#define THOUSAND 1000
#define MILLION 1000000

int main(int argc, char *argv[])
{
  int64_t size = atoi(argv[1])*MILLION;
  int64_t num_primes = 0;
  char *numbers;
  struct timeval start;
  struct rusage ru;
  struct timeval tv;
  struct timeval elapsed;

  gettimeofday(&start, NULL);

  numbers = malloc(size);

  for( int64_t i = 0; i < size; i++ )
    {
      numbers[i] = 1;
    }

  int64_t p = 2;
  num_primes++;
  while( 1 )
    {
      int64_t offset = p - 2; // numbers begins at 2
      while( offset < size )
	{
	  numbers[offset] = 0;
	  offset += p;
	}
      
      offset = p-2;
      while( offset < size && numbers[offset] == 0 )
	{offset++;}
      if( offset == size )
	break;
      p = offset+2;
      num_primes++;

      if( num_primes % (100*THOUSAND) == 0 )
	{
	  getrusage(RUSAGE_SELF, &ru);
	  gettimeofday(&tv, NULL);
	  timersub(&tv, &start, &elapsed);
	  printf("num_primes: %.2f M Primes : %.2fMB : %.2f\n", num_primes / (double)MILLION, ru.ru_maxrss / 1024.0, elapsed.tv_sec+elapsed.tv_usec/(double)MILLION);
	}
    }
  printf("found %" PRId64 " primes\n", num_primes);

  return 0;
}
