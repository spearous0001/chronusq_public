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
#include <cxxapi/options.hpp>
#include <cerr.hpp>

#include <util/threads.hpp>

namespace ChronusQ {


  std::shared_ptr<CQMemManager> CQMiscOptions(std::ostream &out,
    CQInputFile &input) {

    size_t mem     = 256e6; // Default 256 MB allocation
    size_t blkSize = 2048;  // Default 2KB block size

    // Determine if memory allocation was specified
    OPTOPT(
      std::string memStr = input.getData<std::string>("MISC.MEM");
      trim(memStr);

      size_t posKB = memStr.find("KB");
      size_t posMB = memStr.find("MB");
      size_t posGB = memStr.find("GB");

      if( posKB != std::string::npos ) {

        memStr.erase(posKB,2);
        trim(memStr);
        mem = std::stod(memStr) * 1e3;

      } else if( posMB != std::string::npos ) {

        memStr.erase(posMB,2);
        trim(memStr);
        mem = std::stod(memStr) * 1e6;

      } else if( posGB != std::string::npos ) {

        memStr.erase(posGB,2);
        trim(memStr);
        mem = std::stod(memStr) * 1e9;

      } else 
        mem = std::stod(memStr);

    )

    OPTOPT(blkSize = input.getData<size_t>("MISC.MEMBLK");)

    std::string postfixes = " KMGT";
    size_t indx = std::floor(std::log10(mem))/4;
    char postfix = postfixes.c_str()[indx];

    size_t memPrint = (postfix == 'K') ? (mem / 1e3) : 
                      (postfix == 'M') ? (mem / 1e6) : 
                      (postfix == 'G') ? (mem / 1e9) : mem;



    OPTOPT(
      SetNumThreads(input.getData<size_t>("MISC.NSMP"));
    )

    out << "\n\n";

    out << "  *** Allocating " << memPrint << " " << postfix << "B *** \n";
    out << "  *** ChronusQ will use " << GetNumThreads() 
        << " OpenMP threads ***\n\n";
    out << "\n\n";

    return std::make_shared<CQMemManager>(mem,blkSize);

  }; // CQMiscOptions


}; // namespace ChronusQ
