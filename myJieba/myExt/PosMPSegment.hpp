/*******************************
filename: PosMPSegment.hpp
author: wuxian
create time: 20150827
last modified time: 20151126
********************************/
#ifndef CPPJIEBA_POSMPSEGMENT_H
#define CPPJIEBA_POSMPSEGMENT_H

#include <algorithm>
#include <set>
#include <cassert>
#include "../src/Limonp/Logger.hpp"
#include "../src/DictTrie.hpp"
#include "../src/ISegment.hpp"
#include "../src/SegmentBase.hpp"
#include "../myExt/EncodingAdapter.hpp"

namespace CppJieba {

class PosMPSegment: public SegmentBase {
 public:
	 PosMPSegment(const string& dictPath, const string& userDictPath = "") {
    dictTrie_ = new DictTrie(dictPath, userDictPath);
    isNeedDestroy_ = true;
    LogInfo("MPSegment init(%s) ok", dictPath.c_str());
  }
	 PosMPSegment(const DictTrie* dictTrie)
    : dictTrie_(dictTrie), isNeedDestroy_(false) {
    assert(dictTrie_);
  }
  virtual ~PosMPSegment() {
    if(isNeedDestroy_) {
      delete dictTrie_;
    }
  }

  bool isUserDictSingleChineseWord(const Unicode::value_type & value) const {
    return dictTrie_->isUserDictSingleChineseWord(value);
  }

  using SegmentBase::cut;
  virtual bool cut(Unicode::const_iterator begin, Unicode::const_iterator end, vector<string>& res)const {
    vector<Unicode> words;
    words.reserve(end - begin);
    if(!cut(begin, end, words)) {
      return false;
    }
    size_t offset = res.size();
    res.resize(res.size() + words.size());
    for(size_t i = 0; i < words.size(); i++) {
      TransCode::encode(words[i], res[i + offset]);
    }
    return true;
  }

  bool cut(Unicode::const_iterator begin , Unicode::const_iterator end, vector<Unicode>& res) const {
    vector<SegmentChar> segmentChars;

    dictTrie_->find(begin, end, segmentChars);

    calcDP_(segmentChars);

    cut_(segmentChars, res);

    return true;
  }
  const DictTrie* getDictTrie() const {
    return dictTrie_;
  }

 private:
  void calcDP_(vector<SegmentChar>& segmentChars) const {
    size_t nextPos;
    const DictUnit* p;
    double val;

    for(vector<SegmentChar>::reverse_iterator rit = segmentChars.rbegin(); rit != segmentChars.rend(); rit++) {
      rit->pInfo = NULL;
      rit->weight = MIN_DOUBLE;
      assert(!rit->dag.empty());
      for(DagType::const_iterator it = rit->dag.begin(); it != rit->dag.end(); it++) {
        nextPos = it->first;
        p = it->second;
        val = 0.0;
        if(nextPos + 1 < segmentChars.size()) {
          val += segmentChars[nextPos + 1].weight;
        }

        if(p) {
          val += p->weight;
        } else {
          val += dictTrie_->getMinWeight();
        }
        if(val > rit->weight) {
          rit->pInfo = p;
          rit->weight = val;
        }
      }
    }
  }
  void cut_(const vector<SegmentChar>& segmentChars, 
        vector<Unicode>& res) const {
    size_t i = 0;
	string piece;
    while(i < segmentChars.size()) {
      const DictUnit* p = segmentChars[i].pInfo;
	  // cout << "µ±Ç°¼ì²â£º " << EncodingAdapter::UnicodeToUTF8(Unicode(1, segmentChars[i].uniCh)) << endl;
	  while (i < segmentChars.size() && EncodingAdapter::isEnglish(Unicode(1, segmentChars[i].uniCh))) {
		piece += EncodingAdapter::UnicodeToUTF8(Unicode(1, segmentChars[i].uniCh));
		i++;
	  }
	  if (piece.length() != 0) {
		  // cout << "Ó¢ÎÄ×Ö·û£º" << piece << endl;
		  res.push_back(EncodingAdapter::UTF8ToUnicode(piece));
		  piece.clear();
		  continue;
	  }
      if(p) {
        res.push_back(p->word);
        i += p->word.size();
      } else { //single chinese word
        res.push_back(Unicode(1, segmentChars[i].uniCh));
        i++;
      }
    }
  }

 private:
  const DictTrie* dictTrie_;
  bool isNeedDestroy_;
}; // class MPSegment

} // namespace CppJieba

#endif
