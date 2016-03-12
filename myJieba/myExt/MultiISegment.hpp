#ifndef CPPJIEBA_MULTISEGMENTINTERFACE_H
#define CPPJIEBA_MULTISEGMENTINTERFACE_H


namespace CppJieba {
class MultiISegment {
 public:
  virtual ~MultiISegment() {};
  virtual bool cut(Unicode::const_iterator begin , Unicode::const_iterator end, vector<vector<string> >& vres) const = 0;
  virtual bool cut(const string& str, vector<vector<string> >& vres) const = 0;
};
}

#endif
