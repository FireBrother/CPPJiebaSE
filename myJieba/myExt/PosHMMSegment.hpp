/*******************************
filename: PosHMMSegment.hpp
author: wuxian
create time: 20150827
last modified time: 20151126
********************************/
#ifndef CPPJIBEA_POSHMMSEGMENT_H
#define CPPJIBEA_POSHMMSEGMENT_H

#include <iostream>
#include <fstream>
#include <memory.h>
#include <cassert>
#include "PosHMMModel.hpp"
#include "../src/SegmentBase.hpp"
#include "../src/DictTrie.hpp"

namespace CppJieba {

class PosHMMSegment: public SegmentBase {
public:
	PosHMMSegment(const string& folderPath) {
		model_ = new PosHMMModel(folderPath);
	}
	PosHMMSegment(const PosHMMModel* model)
		: model_(model), isNeedDestroy_(false) {
	}
  virtual ~PosHMMSegment() {
    if(isNeedDestroy_) {
      delete model_;
    }
  }

  using SegmentBase::cut;
  bool cut(Unicode::const_iterator begin, Unicode::const_iterator end, vector<Unicode>& res)const {
    Unicode::const_iterator left = begin;
    Unicode::const_iterator right = begin;
    while(right != end) {
      if(*right < 0x80) {
        if(left != right && !cut_(left, right, res)) {
          return false;
        }
        left = right;
        do {
          right = sequentialLetterRule_(left, end);
          if(right != left) {
            break;
          }
          right = numbersRule_(left, end);
          if(right != left) {
            break;
          }
          right ++;
        } while(false);
        res.push_back(Unicode(left, right));
        left = right;
      } else {
        right++;
      }
    }
    if(left != right && !cut_(left, right, res)) {
      return false;
    }
    return true;
  }
  virtual bool cut(Unicode::const_iterator begin, Unicode::const_iterator end, vector<string>& res)const {
    if(begin == end) {
      return false;
    }
    vector<Unicode> words;
    words.reserve(end - begin);
    if(!cut(begin, end, words)) {
      return false;
    }
    size_t offset = res.size();
    res.resize(res.size() + words.size());
    for(size_t i = 0; i < words.size(); i++) {
      TransCode::encode(words[i], res[offset + i]);
    }
    return true;
  }
 private:
  // sequential letters rule
  Unicode::const_iterator sequentialLetterRule_(Unicode::const_iterator begin, Unicode::const_iterator end) const {
    Unicode::value_type x = *begin;
    if (('a' <= x && x <= 'z') || ('A' <= x && x <= 'Z')) {
      begin ++;
    } else {
      return begin;
    }
    while(begin != end) {
      x = *begin;
      if(('a' <= x && x <= 'z') || ('A' <= x && x <= 'Z') || ('0' <= x && x <= '9')) {
        begin ++;
      } else {
        break;
      }
    }
    return begin;
  }
  //
  Unicode::const_iterator numbersRule_(Unicode::const_iterator begin, Unicode::const_iterator end) const {
    Unicode::value_type x = *begin;
    if('0' <= x && x <= '9') {
      begin ++;
    } else {
      return begin;
    }
    while(begin != end) {
      x = *begin;
      if( ('0' <= x && x <= '9') || x == '.') {
        begin++;
      } else {
        break;
      }
    }
    return begin;
  }
  bool cut_(Unicode::const_iterator begin, Unicode::const_iterator end, vector<Unicode>& res) const {
    vector<size_t> status;
    if(!viterbi_(begin, end, status)) {
      LogError("viterbi_ failed.");
      return false;
    }

    Unicode::const_iterator left = begin;
    Unicode::const_iterator right;
    for(size_t i = 0; i < status.size(); i++) {
      if(status[i] % 2) { //if(HMMModel::E == status[i] || HMMModel::S == status[i])
        right = begin + i + 1;
		res.push_back(TransCode::decode("by Hmm:" + string(model_->posMap[PosHMMModel::getStatusPos(status[i]).second])));
        res.push_back(Unicode(left, right));
        left = right;
      }
    }
    return true;
  }

  bool viterbi_(Unicode::const_iterator begin, Unicode::const_iterator end, 
        vector<size_t>& status) const {
    if(begin == end) {
      return false;
    }
	string debugtmp = EncodingAdapter::UTF8ToGBK(TransCode::encode(Unicode(begin, end)));

    size_t Y = PosHMMModel::ALL_STATUS_SUM;
    size_t X = end - begin;

    size_t XYSize = X * Y;
    size_t now, old, stat = PosHMMModel::getStatusValue(PosHMMModel::E, PosHMMModel::x);
    double tmp;

    vector<int> path(XYSize);
    vector<double> weight(XYSize);

	//start
	for (size_t y = 0; y < Y; y++) {
		weight[0 + y * X] = model_->startProb[y] + model_->getEmitProb(model_->emitProb, *begin, y, MIN_DOUBLE);
		path[0 + y * X] = -1;
	}

	double emitProb;

	for (size_t x = 1; x < X; x++) {
		for (size_t y = 0; y < Y; y++) {
			now = x + y*X;
			weight[now] = MIN_DOUBLE;
			path[now] = PosHMMModel::getStatusValue(PosHMMModel::E, PosHMMModel::x); // warning
			emitProb = model_->getEmitProb(model_->emitProb, *(begin + x), y, MIN_DOUBLE);
			for (size_t preY = 0; preY < Y; preY++) {
				old = x - 1 + preY * X;
				tmp = weight[old] + model_->transProb[preY][y] + emitProb;
				if (tmp > weight[now]) {
					weight[now] = tmp;
					path[now] = preY;
				}
			}
		}
	}

	tmp = MIN_DOUBLE;
	for (size_t y = 0; y < Y; y++)
		if (y % 2 && weight[X - 1 + y*X] > tmp) {
			stat = y;
			tmp = weight[X - 1 + y*X];
		}

	vector<pair<PosHMMModel::StatusType, PosHMMModel::POSType> > statusP;
	statusP.resize(X);
    status.resize(X);
    for(int x = X -1 ; x >= 0; x--) {
      status[x] = stat;
	  statusP[x] = PosHMMModel::getStatusPos(stat);
      stat = path[x + stat*X];
    }

    return true;
  }

 private:
  const PosHMMModel* model_;
  bool isNeedDestroy_;
}; // class HMMSegment

} // namespace CppJieba

#endif
