#ifndef CPPJIEBA_APPLICATION_H
#define CPPJIEBA_APPLICATION_H

#include "QuerySegment.hpp"
#include "PosTagger.hpp"
#include "KeywordExtractor.hpp"

namespace CppJieba {

enum CutMethod {
  METHOD_MP,
  METHOD_HMM,
  METHOD_MIX,
  METHOD_FULL,
  METHOD_QUERY
};

class Application {
 public:
  Application(const string& dictPath, 
              const string& modelPath, 
              const string& userDictPath,
			  const string& posDictFolder,
              const string& idfPath,
              const string& stopWordsPath) 
    : dictTrie_(dictPath, userDictPath),
      model_(modelPath),
      mpSeg_(&dictTrie_),
      hmmSeg_(&model_),
      mixSeg_(&dictTrie_, &model_),
      fullSeg_(&dictTrie_),
      querySeg_(&dictTrie_, &model_),
#ifdef POSMIXSEG
      tagger_(dictPath, posDictFolder, userDictPath),
#else
	  tagger_(&dictTrie_, &model_),
#endif
      extractor_(&dictTrie_, 
                 &model_, 
                 idfPath,
                 stopWordsPath) {
  }
  void cut(const string& sentence, vector<string>& words, 
        CutMethod method = METHOD_MIX) const {
    switch(method) {
      case METHOD_MP:
        mpSeg_.cut(sentence, words);
        break;
      case METHOD_HMM:
        hmmSeg_.cut(sentence, words);
        break;
      case METHOD_MIX:
        mixSeg_.cut(sentence, words);
        break;
      case METHOD_FULL:
        fullSeg_.cut(sentence, words);
        break;
      case METHOD_QUERY:
        querySeg_.cut(sentence, words);
        break;
      default:
        LogError("argument method is illegal.");
    }
  }
  bool insertUserWord(const string& word, const string& tag = UNKNOWN_TAG) {
    return dictTrie_.insertUserWord(word, tag);
  }
  void tag(const string& str, vector<vector<pair<string, string> > >& vres) const {
    tagger_.tag(str, vres);
  }
  void extract(const string& str, vector<pair<string, double> >& keyword, 
        size_t topN) const {
    extractor_.extract(str, keyword, topN);
  }
  ~Application() {
  }

 private:
  DictTrie dictTrie_;
  HMMModel model_;
  MPSegment mpSeg_;
  HMMSegment hmmSeg_;
  MixSegment mixSeg_;
  FullSegment fullSeg_;
  QuerySegment querySeg_;
  PosTagger tagger_;
  KeywordExtractor extractor_;
}; // class Application

} // namespace CppJieba

#endif // CPPJIEBA_APPLICATION_H
