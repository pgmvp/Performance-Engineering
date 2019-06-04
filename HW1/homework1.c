#define LEN 10000
#define NTIMES 1000

#include <stdlib.h>
#include <stdio.h>
#include <sys/times.h>
#include <time.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <string.h>
#include <stdint.h>

#include <cblas.h>

float a[LEN] __attribute__((aligned(16)));
float b[LEN] __attribute__((aligned(16)));
float c[LEN] __attribute__((aligned(16)));
float d[LEN] __attribute__((aligned(16)));

float matA[LEN * LEN] __attribute__((aligned(16)));
float matB[LEN * LEN] __attribute__((aligned(16)));
float matC[LEN * LEN] __attribute__((aligned(16)));
float matCorr[LEN * LEN] __attribute__((aligned(16)));

float out[LEN] __attribute__((aligned(16)));

const float sec_const = 1000000.0;

int nothing(float a[LEN], float b[LEN], float c[LEN]){
	return (0);
}

int populate_vec()
{
	for(int i = 0; i < LEN; ++i)
	{
		a[i] = 1;
		b[i] = 1;
		c[i] = 2;
		d[i] = 2;
	}
	
	return 0;
}

int populate_mat()
{
	srand(1);
	
	for(int row = 0; row < LEN; ++row)
	{
		for(int col = 0; col < LEN; ++col)
		{
			int upper = 10;
			int lower = 0;
			
			matA[row * LEN + col] = (rand() % (upper - lower + 1)) + lower;
			matB[row * LEN + col] = (rand() % (upper - lower + 1)) + lower;
		}
	}
}

int equal(float *m1, float *m2)
{
	for(int i = 0; i < LEN; ++i)
	{
		for(int j = 0; j < LEN; ++j)
			if(m1[LEN * i + j] != m2[LEN * i + j])
				return 0;
	}
	
	return 1;
}

int print_mat(float * mat)
{
	for(int row = 0; row < LEN; ++row)
	{
		for(int col = 0; col < LEN; ++col)
		{
			printf("%.2f \t", mat[row * LEN + col]);
		}
		printf("\n");
	}
	
	printf("\n\n");
}

int clear_mat(float *mat)
{
		for(int row = 0; row < LEN; ++row)
			for(int col = 0; col < LEN; ++col)
				mat[row * LEN + col] = 0;
}

int vec_mul()
{

  clock_t start_t;
  clock_t end_t;
  clock_t clock_delta;
  double clock_delta_sec;

  start_t = clock();

  __m128 rA,rB,rC,rD,rAB,rCD,rOut;
  for (int n = 0; n < NTIMES; n++) {
    for (int i = 0; i < LEN; i+=4) {
      rA = _mm_load_ps(&a[i]);
      rB = _mm_load_ps(&b[i]);
	  
	  rC = _mm_load_ps(&c[i]);
      rD = _mm_load_ps(&d[i]);
	  
	  
      rAB = _mm_mul_ps(rA,rB);
	  rCD = _mm_mul_ps(rC,rD);
	  
	  rOut = _mm_add_ps(rAB, rCD);
	  
      _mm_store_ps(&out[i],rOut);
    }
    nothing(a, b, c);
  }
  
  end_t = clock();
  clock_delta = end_t - start_t;
  clock_delta_sec = (double) (clock_delta / sec_const);
  printf("VEC_MUL Call:\t %.2f \t\n", clock_delta_sec);
 
  return 0;
}

int mat_mul(float *a, float *b, float *c)
{

  clock_t start_t;
  clock_t end_t;
  clock_t clock_delta;
  double clock_delta_sec;

  start_t = clock();

  __m128 rA,rB,rC;
  float temp[16] __attribute__((aligned(16)));
	float val = 0;
	
	for(int n = 0; n < NTIMES; ++n)
	{
	for( int row = 0; row < LEN; ++row )
		for( int col = 0; col < LEN; ++col )
		{
			val = 0;
			for ( int k = 0; k < LEN; k+=4 )
			{
				rA = _mm_load_ps(&a[row * LEN + k]);
				rB = _mm_set_ps(b[LEN * (k + 3) + col], b[LEN * (k + 2) + col], b[LEN * (k + 1) + col], b[LEN * k + col]);

				rC = _mm_mul_ps(rA, rB);
				_mm_store_ps(temp,rC);
				val += temp[0] + temp[1] + temp[2] + temp[3];
			}
			c[row * LEN + col] = val;
		}
		nothing(a, b, c);
	}
  
  end_t = clock();
  clock_delta = end_t - start_t;
  clock_delta_sec = (double) (clock_delta / sec_const);
  printf("MAT_MUL Call:\t %.2f \t\n", clock_delta_sec);
  
  return 0;
}

int mat_mul2(float *a, float *b, float *c)
{

  clock_t start_t;
  clock_t end_t;
  clock_t clock_delta;
  double clock_delta_sec;

  start_t = clock();

  __m128 rA,rB,rC;
  float temp[16] __attribute__((aligned(16)));
	float val = 0;
	
	for(int n = 0; n < NTIMES; ++n)
	{
	for( int row = 0; row < LEN; ++row )
		for( int col = 0; col < LEN; ++col )
		{
			val = 0;
			
			//unroll first iteration
			rA = _mm_load_ps(&a[row * LEN]);
			rB = _mm_set_ps(b[LEN * 3 + col], b[LEN * 2 + col], b[LEN * 1 + col], b[col]);
			rC = _mm_mul_ps(rA, rB);
			
			for ( int k = 4; k < LEN; k+=4 )
			{
				rA = _mm_load_ps(&a[row * LEN + k]);
				rB = _mm_set_ps(b[LEN * (k + 3) + col], b[LEN * (k + 2) + col], b[LEN * (k + 1) + col], b[LEN * k + col]);
				
				rC = _mm_add_ps(_mm_mul_ps(rA, rB), rC);
			}
			_mm_store_ps(temp, rC);
			c[row * LEN + col] = temp[0] + temp[1] + temp[2] + temp[3];
		}
		nothing(a, b, c);
	}
  
  end_t = clock();
  clock_delta = end_t - start_t;
  clock_delta_sec = (double) (clock_delta / sec_const);
  printf("MAT_MUL2 Call:\t %.2f \t\n", clock_delta_sec);
  
  return 0;
}

int mat_mul_correct(float *a, float *b, float *c)
{
	int val = 0;
	for( int row = 0; row < LEN; ++row )
		for( int col = 0; col < LEN; ++col )
		{
			val = 0;
			for ( int k = 0; k < LEN; ++k )
				val += a[row * LEN + k] * b[k * LEN + col];
			c[row * LEN + col] = val;
		}
		
	return 0;
}

int mat_mul_blas(float *a, float *b, float *c)
{
	int m = LEN;
    int n = LEN;
    int k = LEN;

    int lda = k;
    int ldb = n;
	float beta = 1.0;
    float alpha = 1.0;
	
	clock_t start_t;
    clock_t end_t;
    clock_t clock_delta;
    double clock_delta_sec;

    start_t = clock();
	
	for (int n = 0; n < NTIMES; ++n)
	{
    // http://www.netlib.org/lapack/explore-html/d7/d2b/dgemm_8f.html
    cblas_sgemm(CblasRowMajor,
                CblasNoTrans, CblasNoTrans, 
                m, n, k, alpha, 
                a, lda,
                b, ldb,
                beta, 
                c, n);
	}
	
   end_t = clock();
   clock_delta = end_t - start_t;
   clock_delta_sec = (double) (clock_delta / sec_const);
   printf("BLAS:\t %.2f \t\n", clock_delta_sec);
   
   return 0;
}

int find_substr_naive(const char* str, const char* substr)
{
	int n = strlen(str);
	int m = strlen(substr);
	
	int val;
	for(int i = 0; i < n - m + 1; ++i)
	{
		int matched = 1;
		for(int j = 0; j < m; ++j)
		{
			if(str[i + j] != substr[j])
			{
				matched = 0;
				break;
			}	
		}
		
		if(matched)
			return i;
	}
	
	return -1;
}

void print128_num(__m128i var)
{
    uint16_t *val = (uint16_t*) &var;
    printf("Numerical: %i %i %i %i %i %i %i %i \n", 
           val[0], val[1], val[2], val[3], val[4], val[5], 
           val[6], val[7]);
}

int find_substr_naive2(const char* str, const char* substr)
{
	int n = strlen(str);
	int m = strlen(substr);
	
	int val;
	for(int i = 0; i < n - m + 1; ++i)
	{
		int matched = 1;
		for(int j = 0; j < m; j+=4)
		{
			__m128i  first = _mm_load_si128((const __m128i*)(str + i + j));
			__m128i second = _mm_load_si128((const __m128i*)(substr + j));
			
			printf("first\n");
			print128_num(first);
			printf("second\n");
			print128_num(second);
			
			__m128i eq = _mm_cmpeq_epi16(first, second);
			
			val = _mm_cvtsi128_si32(eq);
			
			if(!val)
			{
				matched = 0;
				break;
			}
		}
		
		if(matched)
			return i;
	}
	
	return -1;
}


int main()
{
	populate_vec();
	vec_mul();
	
	populate_mat();
	//print_mat(matA);
	//print_mat(matB);
	
    //mat_mul_correct(matA, matB, matCorr);
	
	//mat_mul(matA, matB, matC);
	//print_mat(matC);
	//printf("Matrices equal: %i\n", equal(matC, matCorr));
	//clear_mat(matC);
	
	//mat_mul2(matA, matB, matC);
	//print_mat(matC);
	//printf("Matrices equal: %i\n", equal(matC, matCorr));
	//clear_mat(matC);
	
	//print_mat(matCorr);
	
	//mat_mul_blas(matA, matB, matC);
	//print_mat(matC);
	
	const char str[] = "wwwwwwwwhhhhhhhhzzzssssabcdefgh";
	const char s[] = "zzzzssssabcdefgh";
	int res = find_substr_naive2(str, s);
	printf("Substring at pos: %i", res);
	
	return 0;
}