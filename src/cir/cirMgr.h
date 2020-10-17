/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr() {}
   ~CirMgr();

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const {
      if (gid >= getListSize()) return 0;
      return _gateList[gid];
   }
   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   void simulate();

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();


   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() ;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

   void dfsAIG(CirGate*, vector<CirGate*>&) const;
   void resetFlag() const;

   int getListSize() const { return _header.m + _header.o + 1; }


   // optimize
   void opt(CirGate* g);

private:
   ofstream           *_simLog;

   CirGate **_gateList;

   struct Header {
      string aag;
      unsigned int m;
      unsigned int i;
      unsigned int l;
      unsigned int o;
      unsigned int a;
   };

   Header _header;
   void readHeader(const string& x);
   void readPI(const string& x, const unsigned& line);
   void readPO(const string& x, const unsigned& line, const unsigned& No);
   void readAIG(const string& x, const unsigned& line);
   void readSymbol(const string& x);
   void readComment(const string& x);
   void buildConnection();
   void dfsTravel(CirGate* g);
   void dfs();

   vector<unsigned> _PIOrder;

   GateList dfsList;

   string _comment;
};

#endif // CIR_MGR_H
