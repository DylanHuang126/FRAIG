/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
   /*
   ==================================================
    PO(25)”23GAT$PO”, line 9 =
   ==================================================
   */ 
   string output = getTypeStr();
   output += "(";
   output += to_string(getID());
   output += ")";
   // symbol
   if (getSymbol().length() > 0) {
      output += "\"";
      output += getSymbol();
      output += "\"";
   }
   output += ", line ";
   output += to_string(getLineNo());

   cout << "==================================================" << endl;
   cout << "= ";
   cout << setiosflags(ios::left) << setw(47)
         <<  output
         << resetiosflags(ios::left) << "=" << endl;
   cout << "==================================================" << endl;
}

void
CirGate::reportFanin(int level)
{
   assert (level >= 0);
   cirMgr->resetFlag();
   faninRecursive(level, 0, false);

}
void
CirGate::faninRecursive(int level, int indent, bool inv)
{
   assert (level >= 0);
   // indent
   for (auto i = 0; i < indent; ++i) {
      cout << "  ";
   }
   if (inv) { cout << "!"; }
   cout << getTypeStr() << " " << getID();
   if (level > 0) {
      if (isVisited() && _faninPtr.size() > 0) {
         cout << " (*)" << endl;
         return;
      }
      cout << endl;
      setMark(true);
      for (int i = 0; i < _fanin.size(); ++i) {
         bool v = (_faninInv[i]);
         _faninPtr[i]->faninRecursive(level - 1, indent + 1, v);
      }
   }
   else { cout << endl; }
}

void
CirGate::reportFanout(int level)
{
   assert (level >= 0);
   cirMgr->resetFlag();
   fanoutRecursive(level, 0, false);
}

void
CirGate::fanoutRecursive(int level, int indent, bool inv)
{
   assert (level >= 0);
   // indent
   for (auto i = 0; i < indent; ++i) {
      cout << "  ";
   }
   if (inv) { cout << "!"; }
   cout << getTypeStr() << " " << getID();
   if (level > 0) {
      if (isVisited() && _fanoutPtr.size() > 0) {
         cout << " (*)" << endl;
         return;
      }
      cout << endl;
      setMark(true);
      for (int i = 0; i < _fanoutPtr.size(); ++i) {
         bool v;
         for (auto j = 0; j < _fanoutPtr[i]->_faninPtr.size(); ++j) {
            if (_fanoutPtr[i]->_fanin[j] == this->getID()) {
               v = _fanoutPtr[i]->_faninInv[j];
               break;
            }
         }
         _fanoutPtr[i]->fanoutRecursive(level - 1, indent + 1, v);
      }
   }
   else { cout << endl; }
}

void
CirGate::makeSweep(){
   if(_faninPtr.size() > 0 && _faninPtr[0] != 0)
   {
      GateList::iterator pos = find(_faninPtr[0]->_fanoutPtr.begin(), _faninPtr[0]->_fanoutPtr.end(), this);
      IdList::iterator poss = find(_faninPtr[0]->_fanout.begin(), _faninPtr[0]->_fanout.end(), this->getID());
      if(pos != _faninPtr[0]->_fanoutPtr.end()) { _faninPtr[0]->_fanoutPtr.erase(pos);}
      if(poss != _faninPtr[0]->_fanout.end()) { _faninPtr[0]->_fanout.erase(poss); _faninPtr[0] = 0;}
   }
   if(_faninPtr.size() > 0 && _faninPtr[1] != 0)
   {
      GateList::iterator pos1 = find(_faninPtr[1]->_fanoutPtr.begin(), _faninPtr[1]->_fanoutPtr.end(), this);
      IdList::iterator pos11 = find(_faninPtr[1]->_fanout.begin(), _faninPtr[1]->_fanout.end(), this->getID());
      if(pos1 != _faninPtr[1]->_fanoutPtr.end()) { _faninPtr[1]->_fanoutPtr.erase(pos1);}
      if(pos11 != _faninPtr[1]->_fanout.end()) { _faninPtr[1]->_fanout.erase(pos11); _faninPtr[1] = 0;}
   }

   for(GateList::iterator it = _fanoutPtr.begin(); it != _fanoutPtr.end(); ++it)
   {
      CirGate* next = *it;
      if(next->_faninPtr[0] != nullptr && next->_faninPtr[0]->getID() == this->getID())
      {
         IdList::iterator i = find(_fanout.begin(), _fanout.end(), this->getID());
         if(i != _fanout.end()) { next->_fanin.erase(i); }
         next->_faninPtr[0] = 0;
      }
         
      if(next->_faninPtr[1] != nullptr && next->_faninPtr[1]->getID() == this->getID())
      {
         IdList::iterator i = find(_fanout.begin(), _fanout.end(), this->getID());
         if(i != _fanout.end()) { next->_fanin.erase(i); }
         next->_faninPtr[1] = 0;
      }
         
      
   }
   
}