/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{

   HashMap<HashKey,CirGate*> hash(getHashSize(dfsList.size()));
   for(size_t i = 0; i < dfsList.size(); ++i)
   {
      if(dfsList[i]->isAig())
      {
         unsigned in0 = dfsList[i]->_fanin[0] * 2;
         unsigned in1 = dfsList[i]->_fanin[1] * 2;
         if(dfsList[i]->_faninInv[0]) ++in0;
         if(dfsList[i]->_faninInv[1]) ++in1;
         HashKey k(in0, in1);

         if(!hash.insert(k, dfsList[i]))
         {
            CirGate* g = 0;
            hash.query(k, g);
            cout << "Strashing: " << g->getID() << " merging " << dfsList[i]->getID() << "...\n";

            for(size_t id = 0; id < dfsList[i]->_faninPtr.size(); ++id)
            {
               GateList::iterator it = find(dfsList[i]->_faninPtr[id]->_fanoutPtr.begin(), dfsList[i]->_faninPtr[id]->_fanoutPtr.end(), dfsList[i]);
               if(it != dfsList[i]->_faninPtr[id]->_fanoutPtr.end()) 
                  dfsList[i]->_faninPtr[id]->_fanoutPtr.erase(it);
               
               IdList::iterator idt = find(dfsList[i]->_faninPtr[id]->_fanout.begin(), dfsList[i]->_faninPtr[id]->_fanout.end(), dfsList[i]->getID());
               if(idt != dfsList[i]->_faninPtr[id]->_fanout.end())
                  dfsList[i]->_faninPtr[id]->_fanout.erase(idt);
            }
            
            //cout << "fanin finish\n";
            for(size_t id = 0; id < dfsList[i]->_fanoutPtr.size(); ++id)
            {
               GateList::iterator it = find(dfsList[i]->_fanoutPtr[id]->_faninPtr.begin(), dfsList[i]->_fanoutPtr[id]->_faninPtr.end(), dfsList[i]);
               if(it != dfsList[i]->_fanoutPtr[id]->_faninPtr.end()) 
                  *it = g;
               IdList::iterator idt = find(dfsList[i]->_fanoutPtr[id]->_fanin.begin(), dfsList[i]->_fanoutPtr[id]->_fanin.end(), dfsList[i]->getID());
               if(idt != dfsList[i]->_fanoutPtr[id]->_fanin.end())
                  *idt = g->getID();
               
               g->_fanoutPtr.push_back(dfsList[i]->_fanoutPtr[id]);
               g->_fanout.push_back(dfsList[i]->_fanout[id]);            
            }
            //cout << "fanout finish\n";
            --_header.a;
            delete _gateList[dfsList[i]->getID()]; _gateList[dfsList[i]->getID()] = 0;
            
         }
      }
   }

   resetFlag();
   dfs();
   
}

void
CirMgr::fraig()
{
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
