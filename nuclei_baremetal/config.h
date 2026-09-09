/* the maximum number of bytes to allocate, minimum of 4096 */
#define MAX_MEM (1024 * 1024)
/* Three matrices occupy MAX_MEM/4-sized slots; MAX_MEM must be a power of 4. */
#define MAX_MAT \
  (1UL << (((sizeof(unsigned long) * 8 - 1 - __builtin_clzl(MAX_MEM)) / 2) - 2))
_Static_assert(MAX_MAT * MAX_MAT * sizeof(float) <= MAX_MEM / 4,
               "matrix must fit in its memory slot");
/* the byte count for the next run */
#define NEXT(c) (c + 1)

/* minimum number of repeats, to sample median from */
#define MIN_REPEATS 10
/* maxium number of repeats, executed until more than STOP_TIME has elapsed */
#define MAX_REPEATS 64

/* stop repeats early afer this many cycles have elapsed */
#define STOP_CYCLES (1024*1024*500)

/* validate against reference implementation on the first repetition */
#define VALIDATE 1

/* custom scaling factors for benchmarks, these are used to make sure each
 * benchmark approximately takes the same amount of time. */

#define SCALE_mandelbrot(N) ((N)/10)
#define SCALE_mergelines(N) ((N)/10)

/* benchmark specific configurations */
#define mandelbrot_ITER 100
