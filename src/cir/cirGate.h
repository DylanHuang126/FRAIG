/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

// class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
   friend class cirMgr;

public:
   CirGate(){}
   CirGate(unsigned id, unsigned line, unsigned col): _ID(id), _line(line), _col(col) {}
   virtual ~CirGate() {}

   /// Basic access methods
   virtual string getTypeStr() const = 0;
   unsigned getLineNo() const { return _line; }
   unsigned getColNo() const { return _col; }
   unsigned getID() const { return _ID; }
   virtual bool isAig() const { return false; }


   // Printing functions
   virtual void printGate() const = 0;
   void reportGate() const;
   void reportFanin(int level);
   void reportFanout(int level);

   void setID(const unsigned& gid) { _ID = gid; }
   unsigned getLine() const { return _line; }
   void setLine(const unsigned& l) { _line = l; }
   unsigned getCol() const { return _col; }
   void setCol(const unsigned& c) { _col = c; }

   bool isVisited() const { return _isVisited; }
   void setMark(bool flag) { _isVisited = flag; }

   void addFanout(const unsigned& gid) { _fanout.push_back(gid); }
   vector<unsigned>::iterator findFanoutIndex(const unsigned& gid) {
      return find(_fanout.begin(), _fanout.end(), gid);
   }
   void addFanin(const unsigned& gid) { _fanin.push_back(gid); }
   vector<unsigned>::iterator findFaninIndex(const unsigned& gid) {
      return find(_fanin.begin(), _fanin.end(), gid);
   }

   void addInv(const bool& b) { _faninInv.push_back(b); }

   void setUsed(const bool b) { used = b; }
   bool getUsed() const { return used; }
   vector<unsigned> _fanout;
   vector<unsigned> _fanin;
   vector<bool> _faninInv;
   vector<CirGate*> _faninPtr;
   vector<CirGate*> _fanoutPtr;

   void setSymbol(string s) {
      _s = s;
   }
   string getSymbol() const { return _s; }

   // sweep
   void makeSweep();


private:
   unsigned int _ID;
   unsigned int _line;
   unsigned int _col;
   bool _isVisited = false;
   bool _isReported = false;

   void faninRecursive(int level, int indent, bool inv);
   void fanoutRecursive(int level, int indent, bool inv);

   string _s;

protected:
   bool used = false;
};

class PIGate : public CirGate
{
public:
   PIGate(unsigned id, unsigned line, unsigned col): CirGate(id, line, col) {}
   ~PIGate() {}

   string getTypeStr() const { return "PI"; }
   void printGate() const {
      cout << getTypeStr() << "  " << getID();
      // symbol
      if (getSymbol().length() > 0) {
         cout << " (" << getSymbol() << ")";
      }
      cout << endl;
   }
   
private:
   
};

class POGate : public CirGate
{
public:
   POGate(unsigned id, unsigned line, unsigned col): CirGate(id, line, col) {}
   ~POGate() {}
   
   string getTypeStr() const { return "PO"; }
   void printGate() const {
      cout << getTypeStr() << "  " << getID() << " ";

      int in = _fanin[0];
      if (_faninPtr[0]->getTypeStr() == "UNDEF") cout << "*";
      if (_faninInv[0]) cout << "!";
      cout << in;
      // symbol
      if (getSymbol().length() > 0) {
         cout << " (" << getSymbol() << ")";
      }
      cout << endl;
   }

private:
   
};

class AIGGate : public CirGate
{
public:
   AIGGate(unsigned id, unsigned line, unsigned col): CirGate(id, line, col){}
   ~AIGGate() {}
   
   bool isAig() const { return true; }
   string getTypeStr() const { return "AIG"; }
   void printGate() const {
      cout << getTypeStr() << " " << getID();
      for (int i = 0; i < 2; ++i) {
         cout << " ";
         int in = _fanin[i];
         if (_faninPtr[i]->getTypeStr() == "UNDEF") cout << "*";
         if (_faninInv[i]) cout << "!";
         cout << in;
      }
      cout << endl;
   }

private:

};

class ConstGate : public CirGate
{
public:
   ConstGate(): CirGate(0, 0, 0) {}
   ~ConstGate() {}
   
   string getTypeStr() const { return "CONST"; }
   void printGate() const {
      cout << "CONST0" << endl;
   }

private:
   
};

class UndefGate : public CirGate
{
public:
   UndefGate(const unsigned& id): CirGate(id, 0, 0) {}
   ~UndefGate() {}
   
   string getTypeStr() const { return "UNDEF"; }
   void printGate() const {}

private:
   
};

#endif // CIR_GATE_H
