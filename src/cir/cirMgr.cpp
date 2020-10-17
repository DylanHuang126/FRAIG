/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   ifstream inf(fileName);
   string x;
   unsigned int line = 1;
   getline(inf, x);
   readHeader(x);
   ++line;
   for (auto i = 0; i < _header.i; ++i) {
      getline(inf, x);
      readPI(x, line);
      ++line;
   }
   int PONo = 1;
   for (auto i = 0; i < _header.o; ++i) {
      getline(inf, x);
      readPO(x, line, PONo);
      ++line; 
      ++PONo;
   }
   for (auto i = 0; i < _header.a; ++i) {
      getline(inf, x);
      readAIG(x, line);
      ++line; 
   }

   // connection
   buildConnection();
   
   dfs();
   // symbol
   while (getline(inf, x)) {
      if (x == "c") { break; }
      readSymbol(x);
   }
   // comment
   while (getline(inf, x)) {
      readComment(x);
   }

   // debug
   //cout << "List size: " << getListSize() << endl;
   // for (auto i = 0; i < getListSize(); ++i) {
   //    if (_gateList[i]) {
   //       cout << _gateList[i]->getTypeStr() << endl;
   //       cout << " ID: " << _gateList[i]->getID() << endl;
   //       for (auto j = 0; j < _gateList[i]->_fanin.size(); ++j) {
   //          cout << "  fanin: " << _gateList[i]->_fanin[j] << endl;
   //       }
   //       for (auto j = 0; j < _gateList[i]->_fanout.size(); ++j) {
   //          cout << "  fanout: " << _gateList[i]->_fanout[j] << endl;
   //       }
   //    }
   // }
   inf.close();
   return true;
}

void
CirMgr::dfs() {
   int index = 0;
   dfsList.clear();
   for (auto i = _header.m + 1; i < getListSize(); ++i) {
      dfsTravel(_gateList[i]);
   }
}

void
CirMgr::readHeader(const string& x) {
   vector<string> tokens;
   string token;
   size_t n = myStrGetTok(x, token);
   while (token.size()) {
      tokens.push_back(token);
      n = myStrGetTok(x, token, n);
   }
   assert(tokens.size() == 6);
   _header.aag = tokens[0];
   _header.m = stoi(tokens[1]);
   _header.i = stoi(tokens[2]);
   _header.l = stoi(tokens[3]);
   _header.o = stoi(tokens[4]);
   _header.a = stoi(tokens[5]);

   int arraySize = _header.m + _header.o + 1;
   _gateList = new CirGate*[arraySize];
   for (auto i = 0; i < arraySize; ++i) {
      _gateList[i] = 0;
   }
   _gateList[0] = new ConstGate();
}

void
CirMgr::readPI(const string& x, const unsigned& line) {
   string token;
   size_t n = myStrGetTok(x, token);
   if (n != string::npos) {
      // handle error
      return;
   }
   int id = stoi(token) / 2;
   _gateList[id] = new PIGate(id, line, 1);
   _PIOrder.push_back(id);

}
void
CirMgr::readPO(const string& x, const unsigned& line, const unsigned& No) {
   string token;
   size_t n = myStrGetTok(x, token);
   if (n != string::npos) {
      // handle error
      return;
   }
   int id = _header.m + No;
   
   _gateList[id] = new POGate(id, line, 1);
   
   int id1 = stoi(token);
   if(id1 % 2 == 0){
      _gateList[id]->addFanin(id1/2);
      _gateList[id]->addInv(false);
   }
   if(id1 % 2 == 1){
      _gateList[id]->addFanin(id1/2);
      _gateList[id]->addInv(true);
   }
   

}
void
CirMgr::readAIG(const string& x, const unsigned& line) {
   vector<string> tokens;
   string token;
   size_t n = myStrGetTok(x, token);
   while (token.size()) {
      tokens.push_back(token);
      n = myStrGetTok(x, token, n);
   }
   assert(tokens.size() == 3);

   int id = stoi(tokens[0]) / 2;
   _gateList[id] = new AIGGate(id, line, 1);
   int id1 = stoi(tokens[1]);
   if(id1 % 2 == 0){
      _gateList[id]->addFanin(id1/2);
      _gateList[id]->addInv(false);
   }
   if(id1 % 2 == 1){
      _gateList[id]->addFanin(id1/2);
      _gateList[id]->addInv(true);
   }

   int id2 = stoi(tokens[2]);
   if(id2 % 2 == 0){
      _gateList[id]->addFanin(id2/2);
      _gateList[id]->addInv(false);
   }
   if(id2 % 2 == 1){
      _gateList[id]->addFanin(id2/2);
      _gateList[id]->addInv(true);
   }

   // _gateList[id]->addFanin(stoi(tokens[1]));
   // _gateList[id]->addFanin(stoi(tokens[2]));



}

void
CirMgr::readSymbol(const string& x) {
   string token;
   myStrGetTok(x, token);

   string mode = token.substr(0, 1);
   size_t pos = x.find(' ');
   string s = x.substr(pos+1, x.length() - pos);

   int num = stoi(token.substr(1, token.length() - 1));
   if (mode == "i") {
      int id = _PIOrder[num];
      _gateList[id]->setSymbol(s);
   }
   else if (mode == "o") {
      _gateList[_header.m + num + 1]->setSymbol(s);
   }
}

void
CirMgr::readComment(const string& x) {
   if (!_comment.empty()) _comment += '\n';
   _comment += x;
}
void
CirMgr::buildConnection() {
   for (auto i = 0; i < getListSize(); ++i) {
      if (_gateList[i]) {
         for (auto j = 0; j < _gateList[i]->_fanin.size(); ++j) {
            auto id = _gateList[i]->_fanin[j];
            if (!_gateList[id]) { // undefine
               _gateList[id] = new UndefGate(id);

            }
            _gateList[id]->addFanout(_gateList[i]->getID());
            _gateList[id]->_fanoutPtr.push_back(_gateList[i]);
            _gateList[i]->_faninPtr.push_back(_gateList[id]);
         }
      }
   }
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   cout << endl;
   cout << "Circuit Statistics\n" 
         << "==================\n";
   cout << "  " << setiosflags(ios::left) << setw(5) << "PI" 
            << resetiosflags(ios::left) << setw(9) << _header.i << endl;
   cout << "  " << setiosflags(ios::left) << setw(5) << "PO" 
            << resetiosflags(ios::left) << setw(9) << _header.o << endl;
   cout << "  " << setiosflags(ios::left) << setw(5) << "AIG" 
            << resetiosflags(ios::left) << setw(9) << _header.a << endl;
   cout << "------------------" << endl;
   cout << "  " << setiosflags(ios::left) << setw(5) << "Total" 
            << resetiosflags(ios::left) << setw(9) << _header.i+_header.o+_header.a << endl;
}

void
CirMgr::printNetlist()
{
   resetFlag();
   dfs();
   cout << endl;
   for (unsigned i = 0, n = dfsList.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      dfsList[i]->printGate();
      
   }

   // for(auto i = 0; i < getListSize(); ++i)
   //    cout << *_gateList[i] << endl;

   // cout << endl;
   // resetFlag();
   // int index = 0;
   // for (auto i = _header.m + 1; i < getListSize(); ++i) {
   //    dfsTravel(_gateList[i], index);
   // }
   // cout << dfsList.size() << endl;
   // for (auto i = 0; i < dfsList.size(); ++i) {
   //    cout << dfsList[i]->getID() << endl;
   // }
}

void
CirMgr::dfsTravel(CirGate* g){
   g->setUsed(true);
   if (g->isVisited() || g->getTypeStr() == "UNDEF") return;
   for (auto i = 0; i < g->_fanin.size(); ++i) {
      auto id = g->_fanin[i];
      dfsTravel(_gateList[id]);
   }

   // cout << "[" << index << "] ";
   // g->printGate();
   dfsList.push_back(g);
   g->setMark(true);
}


void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for (auto it = _PIOrder.begin(); it != _PIOrder.end(); ++it) {
      cout << " " << *it;
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for (auto i = _header.m + 1; i < getListSize(); ++i) {
      cout << " " << i;
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   vector<unsigned> fl;
   vector<unsigned> notUsed;
   for (auto i = 1; i < getListSize(); ++i) {
      
      if (_gateList[i]) {
         // float gate
         for (auto j = 0; j < _gateList[i]->_faninPtr.size(); ++j) {
            if (_gateList[i]->_faninPtr[j]->getTypeStr() == "UNDEF") {
               fl.push_back(_gateList[i]->getID());
               break;
            }
         }

         // not used gate
         if (_gateList[i]->_fanoutPtr.size() == 0) {
            if (_gateList[i]->getTypeStr() == "AIG" || _gateList[i]->getTypeStr() == "PI") {
               notUsed.push_back(_gateList[i]->getID());
            }
         }
      }
   }
   if (fl.size() > 0) {
      cout << "Gates with floating fanin(s):";
      for (auto it = fl.begin(); it != fl.end(); ++it) {
         cout << " " << *it;
      }
      cout << endl;
   }
   if (notUsed.size() > 0) {
      cout << "Gates defined but not used  :";
      for (auto it = notUsed.begin(); it != notUsed.end(); ++it) {
         cout << " " << *it;
      }
      cout << endl;
   }
}

void
CirMgr::printFECPairs() const
{
}

void
CirMgr::writeAag(ostream& outfile) const
{
   // header
   outfile << "aag " << _header.m << " " << _header.i 
            << " " << _header.l << " " << _header.o << " ";
   vector<CirGate*> aig;
   resetFlag();
   for (auto i = _header.m + 1; i < getListSize(); ++i) {
      dfsAIG(_gateList[i], aig);
   }
   outfile << aig.size() << endl;

   // PI
   for (int i = 0; i < _PIOrder.size(); ++i) {
      outfile << _PIOrder[i] * 2;
      outfile << endl;
   }

   // PO
   for (int i = _header.m + 1; i < getListSize(); ++i) {
      outfile << _gateList[i]->_fanin[0];
      outfile << endl;  
   }

   // AIG
   for (auto i = 0; i < aig.size(); ++i) {
      outfile << aig[i]->getID() * 2 << " "
            << aig[i]->_fanin[0] << " "
            << aig[i]->_fanin[1] << endl;
   }

   // Symbol
   for (int i = 0; i < _PIOrder.size(); ++i) {
      int id = _PIOrder[i];
      if (_gateList[id]->getSymbol().length() > 0) {
         outfile << "i" << i  << " " << _gateList[id]->getSymbol() << endl;
      }
   }
   for (int i = _header.m + 1; i < getListSize(); ++i) {
      if (_gateList[i]->getSymbol().length() > 0) {
         outfile << "o" << i - (_header.m + 1) << " " << _gateList[i]->getSymbol() << endl;
      }  
   }

   // Comment
   outfile << "c" << endl;
   outfile << "AAG output by Weiting Tang" << endl;
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
}

void 
CirMgr::resetFlag() const {
      for (auto i = 0; i < getListSize(); ++i) {
         if (_gateList[i]) { _gateList[i]->setMark(false); }
      }
   }

void
CirMgr::dfsAIG(CirGate* g, vector<CirGate*>& a) const {
   if (g->isVisited() || g->getTypeStr() == "UNDEF") return;
   for (auto i = 0; i < g->_faninPtr.size(); ++i) {
      dfsAIG(g->_faninPtr[i], a);
   }
   
   if (g->getTypeStr() == "AIG") {
      a.push_back(g);
   }

   g->setMark(true);
}

CirMgr::~CirMgr() {
      for (auto i = 0; i < getListSize(); ++i) {
         if (_gateList[i]) {
            delete _gateList[i];
         }
      }
   }