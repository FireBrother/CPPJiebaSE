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
#include "MultiISegment.hpp"
#include "MultiSegmentBase.hpp"
#include "EncodingAdapter.hpp"

namespace CppJieba {

class PosMPSegment: public MultiSegmentBase {
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

  using MultiSegmentBase::cut;
  virtual bool cut(Unicode::const_iterator begin, Unicode::const_iterator end, vector<vector<string> >& vres)const {
    vector<vector<Unicode> > vwords;
	//vres.reserve(vwords.size());
	//for_each(vwords.begin(), vwords.end(), [&](auto words) { words.reserve(end - begin); });
    if(!cut(begin, end, vwords)) {
      return false;
    }
	vres.resize(vwords.size());
	
	for (int w = 0; w < vres.size() || w < vwords.size(); w++) {
		size_t offset = vres[w].size();
		vres[w].resize(vres[w].size() + vwords[w].size());
		for (size_t i = 0; i < vwords[w].size(); i++) {
			TransCode::encode(vwords[w][i], vres[w][i + offset]);
		}
	}
    return true;
  }

  bool cut(Unicode::const_iterator begin , Unicode::const_iterator end, vector<vector<Unicode> >& vres) const {
	vector<Unicode> res;
    vector<SegmentChar> segmentChars;

    dictTrie_->find(begin, end, segmentChars);

    calcDP_(segmentChars);

    cut_(segmentChars, res);

	for (int i = 0; i < 5; i++)
		vres.push_back(res);
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
