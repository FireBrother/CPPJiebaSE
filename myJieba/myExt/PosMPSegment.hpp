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

	class PosMPSegment : public MultiSegmentBase {
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
			if (isNeedDestroy_) {
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
			if (!cut(begin, end, vwords)) {
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

		bool cut(Unicode::const_iterator begin, Unicode::const_iterator end, vector<vector<Unicode> >& vres) const {
			vector<Unicode> res;
			vector<SegmentChar> segmentChars;

			dictTrie_->find(begin, end, segmentChars);
			vector<MultiSegmentChar> multiSegmentChars;
			for (auto sc : segmentChars)
				multiSegmentChars.push_back(MultiSegmentChar(sc));

			calcDP_(multiSegmentChars);

			cut_(multiSegmentChars, vres);

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

			for (vector<SegmentChar>::reverse_iterator rit = segmentChars.rbegin(); rit != segmentChars.rend(); rit++) {
				rit->pInfo = NULL;
				rit->weight = MIN_DOUBLE;
				assert(!rit->dag.empty());
				for (DagType::const_iterator it = rit->dag.begin(); it != rit->dag.end(); it++) {
					nextPos = it->first;
					p = it->second;
					val = 0.0;
					if (nextPos + 1 < segmentChars.size()) {
						val += segmentChars[nextPos + 1].weight;
					}

					if (p) {
						val += p->weight;
					}
					else {
						val += dictTrie_->getMinWeight();
					}
					if (val > rit->weight) {
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
			while (i < segmentChars.size()) {
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
				if (p) {
					res.push_back(p->word);
					i += p->word.size();
				}
				else { //single chinese word
					res.push_back(Unicode(1, segmentChars[i].uniCh));
					i++;
				}
			}
		}

  struct tuple{size_t u; size_t prev; double w; tuple(size_t _u, size_t _prev, double _w) :u(_u), prev(_prev), w(_w) {} };
  class cmp { public: bool operator () (const tuple&a, const tuple&b) { return a.w < b.w; } };
  void calcDP_(vector<MultiSegmentChar>& multiSegmentChars) const {
	  int k = 0;
	  priority_queue < tuple, deque<tuple>, cmp> pq;
	  vector<double> dis;
	  vector<size_t> prev;
	  dis.resize(multiSegmentChars.size()+1);
	  prev.resize(multiSegmentChars.size()+1);
	  for (int i = 0; i < multiSegmentChars.size(); i++)
		  dis[i] = -100000000;
	  pq.push(tuple(0, 0, 0));
	  while (!pq.empty() && k < MultiSegmentChar::K) {
		  set<size_t> unique;
		  size_t u = pq.top().u;
		  dis[u] = pq.top().w;
		  prev[u] = pq.top().prev;
		  pq.pop();
		  if (u == multiSegmentChars.size()) {
			  cout << "find the a path of weight " << dis[u] << endl;
			  size_t t = multiSegmentChars.size();
			  while (t != 0) {
				  size_t s = prev[t];
				  for (DagType::const_iterator it = multiSegmentChars[s].dag.begin(); it != multiSegmentChars[s].dag.end(); it++) {
					  if (it->first == t - 1) {
						  t = s;
						  multiSegmentChars[s].pInfo[k] = it->second;
						  break;
					  }
				  }
			  }
			  k++;
			  continue;
		  }
		  vector<const DictUnit *> adjmat(multiSegmentChars.size() + 1,(const DictUnit * )0xffffffff);
		  for (DagType::const_iterator it = multiSegmentChars[u].dag.begin(); it != multiSegmentChars[u].dag.end(); it++) {
			  size_t nextPos = it->first + 1;
			  if (adjmat[nextPos] == (const DictUnit *)0xffffffff)
				  adjmat[nextPos] = it->second;
			  else if (adjmat[nextPos]->weight < it->second->weight)
				  adjmat[nextPos] = it->second;
		  }
		  for (size_t nextPos = u + 1; nextPos <= multiSegmentChars.size(); nextPos++) {
			  if (adjmat[nextPos] == (const DictUnit *)0xffffffff) continue;
			  const DictUnit *p = adjmat[nextPos];
			  double val = 0.0;
			  if (p) {
				  val += p->weight;
			  }
			  else {
				  val += dictTrie_->getMinWeight();
			  }
			  pq.push(tuple(nextPos, u, dis[u] + val));
		  }
	  }
  }

  void cut_(const vector<MultiSegmentChar>& multiSegmentChars,
	  vector<vector<Unicode> >& vRes) const {
	  vRes.resize(MultiSegmentChar::K);
	  for (int w = 0; w < MultiSegmentChar::K; w++) {
		  size_t i = 0;
		  string piece;
		  while (i < multiSegmentChars.size()) {
			  const DictUnit* p = multiSegmentChars[i].pInfo[w];
			  // cout << "µ±Ç°¼ì²â£º " << EncodingAdapter::UnicodeToUTF8(Unicode(1, segmentChars[i].uniCh)) << endl;
			  while (i < multiSegmentChars.size() && EncodingAdapter::isEnglish(Unicode(1, multiSegmentChars[i].uniCh))) {
				  piece += EncodingAdapter::UnicodeToUTF8(Unicode(1, multiSegmentChars[i].uniCh));
				  i++;
			  }
			  if (piece.length() != 0) {
				  // cout << "Ó¢ÎÄ×Ö·û£º" << piece << endl;
				  vRes[w].push_back(EncodingAdapter::UTF8ToUnicode(piece));
				  piece.clear();
				  continue;
			  }
			  if (p) {
				  vRes[w].push_back(p->word);
				  i += p->word.size();
			  }
			  else { //single chinese word
				  vRes[w].push_back(Unicode(1, multiSegmentChars[i].uniCh));
				  i++;
			  }
		  }
	  }
  }

 private:
  const DictTrie* dictTrie_;
  bool isNeedDestroy_;
}; // class MPSegment

} // namespace CppJieba

#endif
