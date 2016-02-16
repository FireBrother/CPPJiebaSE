#ifndef CPPJIEBA_POS_TAGGING_H
#define CPPJIEBA_POS_TAGGING_H

#define POSMIXSEG

#include "MixSegment.hpp"
#include "../myExt/PosMixSegment.hpp"
#include "Limonp/StringUtil.hpp"
#include "DictTrie.hpp"

namespace CppJieba {
using namespace Limonp;

static const char* const POS_M = "m";
static const char* const POS_ENG = "eng";
static const char* const POS_X = "x";

class PosTagger {
 public:
  PosTagger(const string& dictPath,
    const string& hmmFolderPath,
    const string& userDictPath = "")
    : segment_(dictPath, hmmFolderPath, userDictPath) {
  }
#ifdef POSMIXSEG
  PosTagger(const DictTrie* dictTrie, const string& hmmFolderPath)
	  : segment_(dictTrie, hmmFolderPath) {
  }
#else
  PosTagger(const DictTrie* dictTrie, const HMMModel* model)
	  : segment_(dictTrie, model) {
  }
#endif
  ~PosTagger() {
  }
#ifdef POSMIXSEG
  bool tag(const string& src, vector<pair<string, string> >& res) const {
	  vector<string> cutRes;
	  if (!segment_.cut(src, cutRes)) {
		  LogError("mixSegment_ cut failed");
		  return false;
	  }

	  const DictUnit *tmp = NULL;
	  Unicode unico;
	  const DictTrie * dict = segment_.getDictTrie();
	  assert(dict != NULL);
	  string POSbyHMM = "";
	  vector<string> v;
	  bool is_space_char = 0;
	  for (vector<string>::iterator itr = cutRes.begin(); itr != cutRes.end(); ++itr) {
		  if (*itr == " ") {
			  // is_space_char = 1;
			  continue;
		  }
		  // if (is_space_char) {
		  //     is_space_char = 0;
		  //     continue;
		  // }
		  if (POSbyHMM != "") {
			  res.push_back(make_pair(*itr, POSbyHMM));
			  POSbyHMM = "";
		  }
		  else {
			  if (startsWith(*itr, "by Hmm")) {
				  split(*itr, v, ":");
				  POSbyHMM = v[1];
			  }
			  else {
				  if (!TransCode::decode(*itr, unico)) {
					  LogError("decode failed.");
					  return false;
				  }
				  tmp = dict->find(unico.begin(), unico.end());
				  if (tmp == NULL || tmp->tag.empty()) {
					  res.push_back(make_pair(*itr, specialRule_(unico)));
				  }
				  else {
					  res.push_back(make_pair(*itr, tmp->tag));
				  }
			  }
		  }
		  
	  }
	  return !res.empty();
  }
#else
  bool tag(const string& src, vector<pair<string, string> >& res) const {
    vector<string> cutRes;
    if (!segment_.cut(src, cutRes)) {
      LogError("mixSegment_ cut failed");
      return false;
    }

    const DictUnit *tmp = NULL;
    Unicode unico;
    const DictTrie * dict = segment_.getDictTrie();
    assert(dict != NULL);
    for (vector<string>::iterator itr = cutRes.begin(); itr != cutRes.end(); ++itr) {
      if (!TransCode::decode(*itr, unico)) {
        LogError("decode failed.");
        return false;
      }
      tmp = dict->find(unico.begin(), unico.end());
      if(tmp == NULL || tmp->tag.empty()) {
        res.push_back(make_pair(*itr, specialRule_(unico)));
      } else {
        res.push_back(make_pair(*itr, tmp->tag));
      }
    }
    return !res.empty();
  }
#endif
 private:
  const char* specialRule_(const Unicode& unicode) const {
    size_t m = 0;
    size_t eng = 0;
    for(size_t i = 0; i < unicode.size() && eng < unicode.size() / 2; i++) {
      if(unicode[i] < 0x80) {
        eng ++;
        if('0' <= unicode[i] && unicode[i] <= '9') {
          m++;
        }
      }
    }
    // ascii char is not found
    if(eng == 0) {
      return POS_X;
    }
    // all the ascii is number char
    if(m == eng) {
      return POS_M;
    }
    // the ascii chars contain english letter
    return POS_ENG;
  }
 private:
  // 使用修改后的混合模型
#ifdef POSMIXSEG
  PosMixSegment segment_;
#else
  MixSegment segment_;
#endif
}; // class PosTagger

} // namespace CppJieba

#endif
