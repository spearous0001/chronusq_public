/* 
 *  This file is part of the Chronus Quantum (ChronusQ) software package
 *  
 *  Copyright (C) 2014-2017 Li Research Group (University of Washington)
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *  
 *  Contact the Developers:
 *    E-Mail: xsli@uw.edu
 *  
 */
#include <cqlinalg/blasext.hpp>

namespace ChronusQ {

  template <typename _F1, typename _F2, typename _F3, typename _FScale1, 
    typename _FScale2>
  void MatAdd(char TRANSA, char TRANSB, size_t M, size_t N, _FScale1 ALPHA, 
    _F1 *A, size_t LDA, _FScale2 BETA, _F2 *B, size_t LDB, _F3 *C, size_t LDC){

      assert( TRANSA == 'N' and TRANSB == 'N' );
      #pragma omp parallel
      {
        _F1 *locA = A;
        _F2 *locB = B;
        _F3 *locC = C;
        #pragma omp for
        for(int j = 0; j < N; j++) {
          #pragma omp simd
          for(int i = 0; i < M; i++)
            locC[i] = ALPHA * locA[i] + BETA * locB[i];
          locA += LDA;
          locB += LDB;
          locC += LDC;
        }
      }

  }; // MatAdd generic template

  template void MatAdd( char, char, size_t, size_t, dcomplex, dcomplex*, 
    size_t, dcomplex, double*, size_t, dcomplex*, size_t);

#ifdef _CQ_MKL

  template<>
  void MatAdd(char TRANSA, char TRANSB, size_t M, size_t N, double ALPHA, 
    double *A, size_t LDA, double BETA, double *B, size_t LDB, double *C, 
    size_t LDC) {

      mkl_domatadd('C',TRANSA,TRANSB,M,N,ALPHA,A,LDA,BETA,B,LDB,C,LDC);

  }; // MatAdd (real, real, real)

  template<>
  void MatAdd(char TRANSA, char TRANSB, size_t M, size_t N, dcomplex ALPHA, 
    dcomplex *A, size_t LDA, dcomplex BETA, dcomplex *B, size_t LDB, 
    dcomplex *C, size_t LDC) {

      mkl_zomatadd('C',TRANSA,TRANSB,M,N,ALPHA,A,LDA,BETA,B,LDB,C,LDC);

  }; // MatAdd (complex, complex, complex)

#else

  template void MatAdd( char, char, size_t, size_t, dcomplex, dcomplex*, 
    size_t, dcomplex, dcomplex*, size_t, dcomplex*, size_t);

  template void MatAdd( char, char, size_t, size_t, double, double*, 
    size_t, double, double*, size_t, double*, size_t);

#endif

}; // namespace ChronusQ