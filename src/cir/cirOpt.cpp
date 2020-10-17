/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include "algorithm"
using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
   for(unsigned i = 1; i < getListSize(); ++i){
      if(_gateList[i] != nullptr && _gateList[i]->getTypeStr() != "PI")
      {
         if(!_gateList[i]->getUsed())
         {
            cout << "Sweeping: " << _gateList[i]->getTypeStr() << "(" << _gateList[i]->getID() << ") removed...\n";
            _gateList[i]->makeSweep();
            if(_gateList[i]->getTypeStr() == "AIG")
               --_header.a;
               
            if(_gateList[i]->getTypeStr() == "PO")
               --_header.o;
            delete _gateList[i];
            _gateList[i] = 0;
         }
      }
   }
   resetFlag();
   dfs();
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
   for(auto i = 0; i < dfsList.size(); ++i)
   {
      opt(dfsList[i]);
   }
   resetFlag();
   dfs();
}

void
CirMgr::opt(CirGate* g)
{
   if(!g->isAig()) return;
   
   unsigned id0 = g->_fanin[0]*2, id1 = g->_fanin[1]*2;
   string s0 = "", s1 = "";
   if(g->_faninInv[0]) { ++id0; s0 = "!"; }
   if(g->_faninInv[1]) { ++id1; s1 = "!"; }
   // has const 1
   if(id0 == 1 || id1 == 1)
   {
      if(id0 == 1 && id1 != 1)
      {
         GateList::iterator it = find(g->_faninPtr[0]->_fanoutPtr.begin(), g->_faninPtr[0]->_fanoutPtr.end(), g);
         if (it != g->_faninPtr[0]->_fanoutPtr.end()) {g->_faninPtr[0]->_fanoutPtr.erase(it);}
         it = find(g->_faninPtr[1]->_fanoutPtr.begin(), g->_faninPtr[1]->_fanoutPtr.end(), g);
         IdList::iterator idt = find(g->_faninPtr[0]->_fanout.begin(), g->_faninPtr[0]->_fanout.end(), g->getID());
         if (idt != g->_faninPtr[0]->_fanout.end()) {g->_faninPtr[0]->_fanout.erase(idt);}
         idt = find(g->_faninPtr[1]->_fanout.begin(), g->_faninPtr[1]->_fanout.end(), g->getID());
         //cout << "finished remove fanin's fanout\n";
         for(auto i = 0; i < g->_fanoutPtr.size(); ++i)
         {
            for(auto j = 0; j < g->_fanoutPtr[i]->_faninPtr.size(); ++j)
            {
               if(g->_fanoutPtr[i]->_faninPtr[j] == g)
               {
                  g->_fanoutPtr[i]->_faninPtr[j] = g->_faninPtr[1];
                  g->_fanoutPtr[i]->_fanin[j] = g->_fanin[1];
                  if( it != g->_faninPtr[1]->_fanoutPtr.end()){ *it = g->_fanoutPtr[i]; }
                  if( idt != g->_faninPtr[1]->_fanout.end()) { *idt = g->_fanout[i]; }
                  // cout << "f fanoutList's fanin sign " << g->_fanoutPtr[i]->_faninInv[j] << endl;
                  if(g->_fanoutPtr[i]->_faninInv[j])
                     g->_fanoutPtr[i]->_faninInv[j] = !g->_faninInv[1];
                  else
                     g->_fanoutPtr[i]->_faninInv[j] = g->_faninInv[1];
                  // cout << "f fanoutList's fanin update to " << g->_faninPtr[1]->getID() << endl;
                  // cout << "f fanoutList's fanin sign update to " << g->_fanoutPtr[i]->_faninInv[j] << endl;
               }
            }
         }
         cout << "Simplifying: " << g->_fanin[1] << " merging " << s1 << g->getID() << "...\n";
         delete _gateList[g->getID()]; _gateList[g->getID()] = 0;
         --_header.a;
         return;
      }
      if(id0 != 1 && id1 == 1)
      {
         GateList::iterator it = find(g->_faninPtr[1]->_fanoutPtr.begin(), g->_faninPtr[1]->_fanoutPtr.end(), g);
         if (it != g->_faninPtr[1]->_fanoutPtr.end()) {g->_faninPtr[1]->_fanoutPtr.erase(it);}
         it = find(g->_faninPtr[0]->_fanoutPtr.begin(), g->_faninPtr[0]->_fanoutPtr.end(), g);
         IdList::iterator idt = find(g->_faninPtr[1]->_fanout.begin(), g->_faninPtr[1]->_fanout.end(), g->getID());
         if (idt != g->_faninPtr[1]->_fanout.end()) {g->_faninPtr[1]->_fanout.erase(idt);}
         idt = find(g->_faninPtr[0]->_fanout.begin(), g->_faninPtr[0]->_fanout.end(), g->getID());
         for(auto i = 0; i < g->_fanoutPtr.size(); ++i)
         {
            for(auto j = 0; j < g->_fanoutPtr[i]->_faninPtr.size(); ++j)
            {
               if(g->_fanoutPtr[i]->_faninPtr[j] == g)
               {
                  g->_fanoutPtr[i]->_faninPtr[j] = g->_faninPtr[0];
                  g->_fanoutPtr[i]->_fanin[j] = g->_fanin[0];
                  if( it != g->_faninPtr[0]->_fanoutPtr.end()){ *it = g->_fanoutPtr[i]; }
                  if( idt != g->_faninPtr[0]->_fanout.end()) { *idt = g->_fanout[i]; }
                  if(g->_fanoutPtr[i]->_faninInv[j])
                     g->_fanoutPtr[i]->_faninInv[j] = !g->_faninInv[0];
                  else
                     g->_fanoutPtr[i]->_faninInv[j] = g->_faninInv[0];
                  // cout << "f fanoutList's fanin update to " << g->_faninPtr[0]->getID() << endl;
                  // cout << "f fanoutList's fanin sign update to " << g->_fanoutPtr[i]->_faninInv[j] << endl;
               }
            }
         }
         cout << "Simplifying: " << g->_fanin[0] << " merging " << s0 << g->getID() << "...\n";
         delete _gateList[g->getID()]; _gateList[g->getID()] = 0;
         --_header.a;
         return;
      }
   }

   // has const 0
   if(id0 == 0 || id1 == 0)
   {
      if(id0 == 0 && id1 != 0)
      {
         GateList::iterator it = find(g->_faninPtr[1]->_fanoutPtr.begin(), g->_faninPtr[1]->_fanoutPtr.end(), g);
         if (it != g->_faninPtr[1]->_fanoutPtr.end()) {g->_faninPtr[1]->_fanoutPtr.erase(it);}
         it = find(g->_faninPtr[0]->_fanoutPtr.begin(), g->_faninPtr[0]->_fanoutPtr.end(), g);
         IdList::iterator idt = find(g->_faninPtr[1]->_fanout.begin(), g->_faninPtr[1]->_fanout.end(), g->getID());
         if (idt != g->_faninPtr[1]->_fanout.end()) {g->_faninPtr[1]->_fanout.erase(idt);}
         idt = find(g->_faninPtr[0]->_fanout.begin(), g->_faninPtr[0]->_fanout.end(), g->getID());
         for(auto i = 0; i < g->_fanoutPtr.size(); ++i)
         {
            for(auto j = 0; j < g->_fanoutPtr[i]->_faninPtr.size(); ++j)
            {
               if(g->_fanoutPtr[i]->_faninPtr[j] == g)
               {
                  g->_fanoutPtr[i]->_faninPtr[j] = g->_faninPtr[0];
                  g->_fanoutPtr[i]->_fanin[j] = g->_fanin[0];
                  if( it != g->_faninPtr[0]->_fanoutPtr.end()){ *it = g->_fanoutPtr[i]; }
                  if( idt != g->_faninPtr[0]->_fanout.end()) { *idt = g->_fanout[i]; }
                  if(g->_fanoutPtr[i]->_faninInv[j])
                     g->_fanoutPtr[i]->_faninInv[j] = !g->_faninInv[0];
                  else
                     g->_fanoutPtr[i]->_faninInv[j] = g->_faninInv[0];
                  // cout << "f fanoutList's fanin update to " << g->_faninPtr[0]->getID() << endl;
                  // cout << "f fanoutList's fanin sign update to " << g->_fanoutPtr[i]->_faninInv[j] << endl;
               }
            }
         }
         cout << "Simplifying: " << g->_fanin[0] << " merging " << s1 << g->getID() << "...\n";
         delete _gateList[g->getID()]; _gateList[g->getID()] = 0;
         --_header.a;
         return;
      }
      if(id0 != 0 && id1 == 0)
      {
         GateList::iterator it = find(g->_faninPtr[0]->_fanoutPtr.begin(), g->_faninPtr[0]->_fanoutPtr.end(), g);
         if (it != g->_faninPtr[0]->_fanoutPtr.end()) {g->_faninPtr[0]->_fanoutPtr.erase(it);}
         it = find(g->_faninPtr[1]->_fanoutPtr.begin(), g->_faninPtr[1]->_fanoutPtr.end(), g);
         IdList::iterator idt = find(g->_faninPtr[0]->_fanout.begin(), g->_faninPtr[0]->_fanout.end(), g->getID());
         if (idt != g->_faninPtr[0]->_fanout.end()) {g->_faninPtr[0]->_fanout.erase(idt);}
         idt = find(g->_faninPtr[1]->_fanout.begin(), g->_faninPtr[1]->_fanout.end(), g->getID());
         for(auto i = 0; i < g->_fanoutPtr.size(); ++i)
         {
            for(auto j = 0; j < g->_fanoutPtr[i]->_faninPtr.size(); ++j)
            {
               if(g->_fanoutPtr[i]->_faninPtr[j] == g)
               {
                  g->_fanoutPtr[i]->_faninPtr[j] = g->_faninPtr[1];
                  g->_fanoutPtr[i]->_fanin[j] = g->_fanin[1];
                  if( it != g->_faninPtr[1]->_fanoutPtr.end()){ *it = g->_fanoutPtr[i]; }
                  if( idt != g->_faninPtr[1]->_fanout.end()) { *idt = g->_fanout[i]; }
                  if(g->_fanoutPtr[i]->_faninInv[j])
                     g->_fanoutPtr[i]->_faninInv[j] = !g->_faninInv[1];
                  else
                     g->_fanoutPtr[i]->_faninInv[j] = g->_faninInv[1];
                  // cout << "f fanoutList's fanin update to " << g->_faninPtr[1]->getID() << endl;
                  // cout << "f fanoutList's fanin sign update to " << g->_fanoutPtr[i]->_faninInv[j] << endl;
               }
            }
         }
         cout << "Simplifying: " << g->_fanin[1] << " merging " << s0 << g->getID() << "...\n";
         delete _gateList[g->getID()]; _gateList[g->getID()] = 0;
         --_header.a;
         return;
      }
   }

   // identical fanin
   if(id0 == id1)
   {
      //cout << "has same fanin\n";
      for(auto i = 0; i < g->_fanoutPtr.size(); ++i)
      {
         for(auto j = 0; j < g->_fanoutPtr[i]->_faninPtr.size(); ++j)
         {
            if(g->_fanoutPtr[i]->_faninPtr[j] == g)
            {
               g->_fanoutPtr[i]->_faninPtr[j] = g->_faninPtr[0];
               g->_fanoutPtr[i]->_fanin[j] = g->_fanin[0];
               // if( it != g->_faninPtr[0]->_fanoutPtr.end()){ *it = g->_fanoutPtr[i]; }
               // if( idt != g->_faninPtr[0]->_fanout.end()) { *idt = g->_fanout[i]; }
               g->_faninPtr[0]->_fanoutPtr.push_back(g->_fanoutPtr[i]);
               g->_faninPtr[0]->_fanout.push_back(g->_fanout[i]);
               if(g->_fanoutPtr[i]->_faninInv[j])
                  g->_fanoutPtr[i]->_faninInv[j] = !g->_faninInv[0];
               else
                  g->_fanoutPtr[i]->_faninInv[j] = g->_faninInv[0];
               // cout << "f fanoutList's fanin update to " << g->_faninPtr[0]->getID() << endl;
               // cout << "f fanoutList's fanin sign update to " << g->_fanoutPtr[i]->_faninInv[j] << endl;
            }
         }
      }
      GateList::iterator it = find(g->_faninPtr[0]->_fanoutPtr.begin(), g->_faninPtr[0]->_fanoutPtr.end(), g);
      IdList::iterator idt = find(g->_faninPtr[0]->_fanout.begin(), g->_faninPtr[0]->_fanout.end(), g->getID());
      if( it != g->_faninPtr[0]->_fanoutPtr.end()){ g->_faninPtr[0]->_fanoutPtr.erase(it); }
      if( idt != g->_faninPtr[0]->_fanout.end()) { g->_faninPtr[0]->_fanout.erase(idt); }
      it = find(g->_faninPtr[0]->_fanoutPtr.begin(), g->_faninPtr[0]->_fanoutPtr.end(), g);
      idt = find(g->_faninPtr[0]->_fanout.begin(), g->_faninPtr[0]->_fanout.end(), g->getID());
      if( it != g->_faninPtr[0]->_fanoutPtr.end()){ g->_faninPtr[0]->_fanoutPtr.erase(it); }
      if( idt != g->_faninPtr[0]->_fanout.end()) { g->_faninPtr[0]->_fanout.erase(idt); }
      //cout << g->_faninPtr[0]->_fanoutPtr.size() << " " <<  g->_faninPtr[0]->_fanout.size() << endl;
      
      cout << "Simplifying: " << g->_fanin[0] << " merging " << s0 << g->getID() << "...\n";
      delete _gateList[g->getID()]; _gateList[g->getID()] = 0;
      --_header.a;
      return;
   }

   // inverted fanin
   if(g->_faninInv[0] != g->_faninInv[1] && g->_fanin[0] == g->_fanin[1])
   {
      //cout << "has inverted fanin\n";
      GateList::iterator it = find(g->_faninPtr[0]->_fanoutPtr.begin(), g->_faninPtr[0]->_fanoutPtr.end(), g);
      if (it != g->_faninPtr[0]->_fanoutPtr.end()) { g->_faninPtr[0]->_fanoutPtr.erase(it); }
      it = find(g->_faninPtr[1]->_fanoutPtr.begin(), g->_faninPtr[1]->_fanoutPtr.end(), g);
      if (it != g->_faninPtr[1]->_fanoutPtr.end()) {g->_faninPtr[1]->_fanoutPtr.erase(it); }
      IdList::iterator idt = find(g->_faninPtr[0]->_fanout.begin(), g->_faninPtr[0]->_fanout.end(), g->getID());
      if (idt != g->_faninPtr[0]->_fanout.end()) { g->_faninPtr[0]->_fanout.erase(idt); }
      idt = find(g->_faninPtr[1]->_fanout.begin(), g->_faninPtr[1]->_fanout.end(), g->getID());
      if (idt != g->_faninPtr[1]->_fanout.end()) { g->_faninPtr[1]->_fanout.erase(idt); }
      
      for(auto i = 0; i < g->_fanoutPtr.size(); ++i)
      {
         for(auto j = 0; j < g->_fanoutPtr[i]->_faninPtr.size(); ++j)
         {
            if(g->_fanoutPtr[i]->_faninPtr[j] == g)
            {
               g->_fanoutPtr[i]->_faninPtr[j] = _gateList[0];
               g->_fanoutPtr[i]->_fanin[j] = 0;
               // cout << "f fanoutList's fanin update to CONST0\n";
               // cout << "f fanoutList's fanin sign update to " << g->_fanoutPtr[i]->_faninInv[j] << endl;
            }
         }
      }
      cout << "Simplifying: " << _gateList[0]->getID() << " merging " << g->getID() << "...\n";
      delete _gateList[g->getID()]; _gateList[g->getID()] = 0;
      --_header.a;
      return;
   }

}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
