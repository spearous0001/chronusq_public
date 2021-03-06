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
#ifndef __INCLUDED_SINGLESLATER_FOCK_HPP__
#define __INCLUDED_SINGLESLATER_FOCK_HPP__

#include <singleslater.hpp>
#include <cqlinalg/blasext.hpp>

namespace ChronusQ {

  /**
   *  \brief Forms the Fock matrix for a single slater determinant using
   *  the 1PDM.
   *
   *  \param [in] increment Whether or not the Fock matrix is being 
   *  incremented using a previous density
   *
   *  Populates / overwrites fock strorage
   */ 
  template <typename T>
  void SingleSlater<T>::formFock(EMPerturbation &pert, bool increment, double xHFX) {

    size_t NB = aoints.basisSet().nBasis;
    size_t NB2 = NB*NB;

    // Form G[D]
    formGD(increment,xHFX);

    // Zero out the Fock
    for(auto &F : fock) std::fill_n(F,NB2,0.);

    // Copy over the Core Hamiltonian
    SetMatRE('N',NB,NB,1.,aoints.coreH[SCALAR],NB,fock[SCALAR],NB);
    for(auto i = 1; i < aoints.coreH.size(); i++) 
      SetMatIM('N',NB,NB,1.,aoints.coreH[i],NB,fock[i],NB);

    // Add in the perturbation tensor
    for(auto i = 0ul; i < fock.size(); i++)
      MatAdd('N','N', NB, NB, T(1.), fock[i], NB, T(1.), GD[i], NB,
        fock[i], NB);


    if(pert.fields.size() != 0) {
      auto dipole = pert.getAmp();

      for(auto iXYZ = 0; iXYZ < 3; iXYZ++)
        if(std::abs(dipole[iXYZ]) > 1e-10)
          MatAdd('N','N', NB, NB, T(1.), fock[0], NB, T(-2*dipole[iXYZ]), 
            this->aoints.lenElecDipole[iXYZ], NB, fock[0], NB);
    };

#if 0
    printFock(std::cout);
#endif

  }; // SingleSlater<T>::fockFock


  /**
   *  \brief Forms the Hartree-Fock perturbation tensor
   *
   *  Populates / overwrites GD storage (and JScalar and K storage)
   */ 
  template <typename T>
  void SingleSlater<T>::formGD(bool increment, double xHFX) {

    // Decide list of onePDMs to use
    oper_t_coll &contract1PDM  = increment ? deltaOnePDM : this->onePDM;

    size_t NB = aoints.basisSet().nBasis;
    size_t NB2 = NB*NB;

    // Possibly allocate a temporary for J matrix
    T* JContract;
    if(std::is_same<double,T>::value) 
      JContract = reinterpret_cast<T*>(JScalar);
    else {
      JContract = this->memManager.template malloc<T>(NB2);
    }

    // Zero out J
    if(not increment or not std::is_same<double,T>::value)
      memset(JContract,0,NB2*sizeof(T));

    std::vector<TwoBodyContraction<T,T>> contract =
      { {contract1PDM[SCALAR], JContract, true, COULOMB} };

    // Determine how many (if any) exchange terms to calculate
    if( std::abs(xHFX) > 1e-12 )
    for(auto i = 0; i < K.size(); i++) {
      contract.push_back({contract1PDM[i], K[i], true, EXCHANGE});

      // Zero out K[i]
      if(not increment) memset(K[i],0,NB2*sizeof(T));
    }

    aoints.twoBodyContract(contract);

    if(not std::is_same<double,T>::value) {
      if(not increment)
        GetMatRE('N',NB,NB,1.,JContract,NB,JScalar,NB);
      else {
        MatAdd('N','N',NB,NB,T(1.),JContract,NB,T(1.),
          JScalar,NB,JContract,NB);
        GetMatRE('N',NB,NB,1.,JContract,NB,JScalar,NB);
      }
      this->memManager.free(JContract);
    }

    // Form GD: G[D] = 2.0*J[D] - K[D]

    if( std::abs(xHFX) > 1e-12 )
    for(auto i = 0; i < K.size(); i++)
      MatAdd('N','N', NB, NB, T(0.), GD[i], NB, T(-xHFX), K[i], NB,
        GD[i], NB);
    else
    for(auto i = 0; i < fock.size(); i++)
      memset(GD[i],0,NB2*sizeof(T));
    
    

    // G[D] += 2*J[D]
    MatAdd('N','N', NB, NB, T(1.), GD[SCALAR], NB, T(2.), JScalar, NB,
      GD[SCALAR], NB);
      
#if 0
    printJ(std::cout);
    printK(std::cout);
    printGD(std::cout);
#endif

  }; // SingleSlater<T>::formGD

}; // namespace ChronusQ

#endif
