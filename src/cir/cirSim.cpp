/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
   // for(size_t i = 0; i < dfsList.size(); ++i)
   // {
   //    if(dfsList[i]->isAig())
   //    {

   //    }
   // }
}

void
CirMgr::fileSim(ifstream& patternFile)
{
   string line;
   vector<size_t>* simP = new vector<size_t> [_header.i];
   
   while(patternFile >> line)
   {
      size_t t = 0;
      if(patternFile.eof()) break;
   
      if(line.size() != _header.i)
      {
         cerr << "Error: Pattern(" << line << ") length(" << line.size();
         cerr << ") does not match the number of inputs(" << _header.i << ") in a circuit!!";
      }

   }
}

// void
// CirMgr::simulate()
// {

// }

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

