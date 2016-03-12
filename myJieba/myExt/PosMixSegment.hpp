/*******************************
filename: PosMixSegment.hpp
author: wuxian
create time: 20150827
last modified time: 20151126
********************************/
#ifndef CPPJIEBA_POSMIXSEGMENT_H
#define CPPJIEBA_POSMIXSEGMENT_H

#include <cassert>
#include "PosMPSegment.hpp"
#include "PosHMMSegment.hpp"
#include "../src/Limonp/StringUtil.hpp"

namespace CppJieba {
class PosMixSegment: public MultiSegmentBase {
public:
	PosMixSegment(const string& mpSegDict, const string& hmmSegDictFolder,
		const string& userDict = "")
		: mpSeg_(mpSegDict, userDict),
		hmmSeg_(hmmSegDictFolder) {
		LogInfo("PosMixSegment init %s, %s", mpSegDict.c_str(), hmmSegDictFolder.c_str());
	}
	PosMixSegment(const DictTrie* dictTrie, const PosHMMModel* model)
		: mpSeg_(dictTrie), hmmSeg_(model) {
	}
	PosMixSegment(const DictTrie* dictTrie, const string& hmmSegDictFolder)
		: mpSeg_(dictTrie), hmmSeg_(hmmSegDictFolder) {
		LogInfo("PosMixSegment init %s", hmmSegDictFolder.c_str());
	}
  virtual ~PosMixSegment() {
  }
  using MultiSegmentBase::cut;
  virtual bool cut(Unicode::const_iterator begin, Unicode::const_iterator end, vector<vector<Unicode> >& vres) const {
    vector<vector<Unicode> > vwords;
	//vwords.reserve(vres.size());
	//for_each(vwords.begin(), vwords.end(), [&](auto words) { words.reserve(end - begin); });
	//return hmmSeg_.cut(begin, end, res);
    if(!mpSeg_.cut(begin, end, vwords)) {
      LogError("mpSeg cutDAG failed.");
      return false;
    }
	vres.reserve(vwords.size());
	for (int i = vres.size(); i < vwords.size(); i++)
		vres.push_back(vector<Unicode>());

    vector<Unicode> hmmRes;
    hmmRes.reserve(end - begin);
    Unicode piece;
    piece.reserve(end - begin);

	Unicode buff;
	buff.reserve(end - begin);
	for (int w = 0; w < vwords.size(); w++) {
		for (size_t i = 0, j = 0; i < vwords[w].size(); i++) {
			//if mp get a word, it's ok, put it into result
			if (1 != vwords[w][i].size() || (vwords[w][i].size() == 1 && mpSeg_.isUserDictSingleChineseWord(vwords[w][i][0]))) {
				vres[w].push_back(vwords[w][i]);
				continue;
			}

			// if mp get a single one and it is not in userdict, collect it in sequence
			j = i;
			while (j < vwords[w].size() && 1 == vwords[w][j].size() && !mpSeg_.isUserDictSingleChineseWord(vwords[w][j][0])) {
				piece.push_back(vwords[w][j][0]);
				j++;
			}

			// cout << "ÊÕ¼¯µ½Î´Í×ÉÆÇÐ·Ö×Ö·û´®£º" << EncodingAdapter::UnicodeToUTF8(piece) << endl;

			// cut the sequence with hmm
			if (!hmmSeg_.cut(piece.begin(), piece.end(), hmmRes)) {
				LogError("hmmSeg_ cut failed.");
				return false;
			}

			//put hmm result to result
			for (size_t k = 0; k < hmmRes.size(); k++) {
				vres[w].push_back(hmmRes[k]);
			}

			//clear tmp vars
			piece.clear();
			hmmRes.clear();

			//let i jump over this piece
			i = j - 1;
		}
	}
    return true;
  }

  virtual bool cut(Unicode::const_iterator begin, Unicode::const_iterator end, vector<vector<string> >& vres)const {
    if(begin == end) {
      return false;
    }

	vector<vector<Unicode> > vuRes;
	cut(begin, end, vuRes);
	vres.resize(vuRes.size());

	for (int w = 0; w < vuRes.size(); w++) {
		size_t offset = vres[w].size();
		vres[w].resize(vres[w].size() + vuRes[w].size());
		for (size_t i = 0; i < vuRes[w].size(); i++, offset++) {
			TransCode::encode(vuRes[w][i], vres[w][offset]);
		}
	}
    return true;
  }

  const DictTrie* getDictTrie() const {
    return mpSeg_.getDictTrie();
  }
 private:
  PosMPSegment mpSeg_;
  PosHMMSegment hmmSeg_;

}; // class MixSegment

} // namespace CppJieba

#endif
