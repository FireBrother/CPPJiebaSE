#ifndef CPPJIEBA_MULTISEGMENTBASE_H
#define CPPJIEBA_MULTISEGMENTBASE_H

#include "../src/TransCode.hpp"
#include "../src/Limonp/Logger.hpp"
#include "../src/Limonp/NonCopyable.hpp"
#include "MultiISegment.hpp"
#include <cassert>


namespace CppJieba {
using namespace Limonp;
/*
//const char* const SPECIAL_CHARS = " \t\n";
#ifndef CPPJIEBA_GBK
const UnicodeValueType SPECIAL_SYMBOL[] = {32u, 9u, 10u, 12290u, 65292u};
#else
const UnicodeValueType SPECIAL_SYMBOL[] = {32u, 9u, 10u};
#endif
*/
class MultiSegmentBase: public MultiISegment, public NonCopyable {
 public:
  MultiSegmentBase() {
    loadSpecialSymbols_();
  };
  virtual ~MultiSegmentBase() {};
 public:
  virtual bool cut(Unicode::const_iterator begin, Unicode::const_iterator end, vector<vector<string> >& vres) const = 0;
  virtual bool cut(const string& str, vector<vector<string> >& vres) const {
    vres.clear();

    Unicode unicode;
    unicode.reserve(str.size());

    TransCode::decode(str, unicode);

    Unicode::const_iterator left = unicode.begin();
    Unicode::const_iterator right;

    for(right = unicode.begin(); right != unicode.end(); right++) {
      if(isIn(specialSymbols_, *right)) {
        if(left != right) {
          cut(left, right, vres);
        }
		for (vector<vector<string> >::iterator itr = vres.begin(); itr != vres.end(); ++itr) {
			itr->resize(itr->size() + 1);
			TransCode::encode(right, right + 1, itr->back());
		}
        left = right + 1;
      }
    }
    if(left != right) {
      cut(left, right, vres);
    }

    return true;
  }
 private:
  void loadSpecialSymbols_() {
    size_t size = sizeof(SPECIAL_SYMBOL)/sizeof(*SPECIAL_SYMBOL);
    for(size_t i = 0; i < size; i ++) {
      specialSymbols_.insert(SPECIAL_SYMBOL[i]);
    }
    assert(specialSymbols_.size());
  }
 private:
  unordered_set<UnicodeValueType> specialSymbols_;

};
}

#endif
