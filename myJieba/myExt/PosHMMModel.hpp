/*******************************
filename: PosHMMModel.hpp
author: wuxian
create time: 20150827
last modified time: 20150828
********************************/
#ifndef CPPJIEBA_POSHMMMODEL_H
#define CPPJIEBA_POSHMMMODEL_H

#include "../src/Limonp/StringUtil.hpp"
#include "../src/Limonp/Logger.hpp"
#include "../src/TransCode.hpp"
#include "../myExt/EncodingAdapter.hpp"

namespace CppJieba {
	ifstream fsopen(string path) {
		ifstream fs(path);
		if (!fs.is_open()) {
			LogFatal("open %s failed.", path.c_str());
		}
		return fs;
	}

	using namespace Limonp;

	struct PosHMMModel {
		/*
		* STATUS:
		* 0: PosHMMModel::B, 1: PosHMMModel::E, 2: PosHMMModel::M, 3:PosHMMModel::S
		* */
		enum StatusType { B = 0, E = 1, M = 2, S = 3, STATUS_SUM = 4 };
		enum POSType { a = 0, ad = 1, ag = 2, an = 3, b = 4, bg = 5, c = 6, d = 7, df = 8, dg = 9, e = 10, en = 11, f = 12, g = 13, h = 14, i = 15, in = 16, j = 17, jn = 18, k = 19, l = 20, ln = 21, m = 22, mg = 23, mq = 24, n = 25, ng = 26, nr = 27, nrfg = 28, nrt = 29, ns = 30, nt = 31, nz = 32, o = 33, p = 34, q = 35, qe = 36, qg = 37, r = 38, rg = 39, rr = 40, rz = 41, s = 42, t = 43, tg = 44, u = 45, ud = 46, ug = 47, uj = 48, ul = 49, uv = 50, uz = 51, v = 52, vd = 53, vg = 54, vi = 55, vn = 56, vq = 57, w = 58, x = 59, y = 60, yg = 61, z = 62, zg = 63, POS_SUM = 64 };
		enum { ALL_STATUS_SUM = StatusType::STATUS_SUM * POSType::POS_SUM };
		static char statMap[STATUS_SUM];
		static char posMap[64][5];
		unordered_map<char, StatusType> status;
		unordered_map<string, POSType> POS;
		typedef uint16_t StatusValueType;
		typedef unordered_map<UnicodeValueType, unordered_map<StatusValueType, double> > EmitProbMapType;
		inline static StatusValueType getStatusValue(StatusType s, POSType p) { return s + p * StatusType::STATUS_SUM; }
		inline static pair<StatusType, POSType> getStatusPos(StatusValueType sv) { return make_pair(StatusType(sv % 4), POSType(sv / 4)); }

		PosHMMModel(const string& modelFolder) {
			memset(startProb, 0, sizeof(startProb));
			for (size_t i = 0; i < ALL_STATUS_SUM; i++)
				for (size_t j = 0; j < ALL_STATUS_SUM; j++)
					transProb[i][j] = MIN_DOUBLE;
			emitProb.clear();
			status['B'] = B;
			status['E'] = E;
			status['M'] = M;
			status['S'] = S;
			POS["a"] = a;
			POS["ad"] = ad;
			POS["ag"] = ag;
			POS["an"] = an;
			POS["b"] = b;
			POS["bg"] = bg;
			POS["c"] = c;
			POS["d"] = d;
			POS["df"] = df;
			POS["dg"] = dg;
			POS["e"] = e;
			POS["en"] = en;
			POS["f"] = f;
			POS["g"] = g;
			POS["h"] = h;
			POS["i"] = i;
			POS["in"] = in;
			POS["j"] = j;
			POS["jn"] = jn;
			POS["k"] = k;
			POS["l"] = l;
			POS["ln"] = ln;
			POS["m"] = m;
			POS["mg"] = mg;
			POS["mq"] = mq;
			POS["n"] = n;
			POS["ng"] = ng;
			POS["nr"] = nr;
			POS["nrfg"] = nrfg;
			POS["nrt"] = nrt;
			POS["ns"] = ns;
			POS["nt"] = nt;
			POS["nz"] = nz;
			POS["o"] = o;
			POS["p"] = p;
			POS["q"] = q;
			POS["qe"] = qe;
			POS["qg"] = qg;
			POS["r"] = r;
			POS["rg"] = rg;
			POS["rr"] = rr;
			POS["rz"] = rz;
			POS["s"] = s;
			POS["t"] = t;
			POS["tg"] = tg;
			POS["u"] = u;
			POS["ud"] = ud;
			POS["ug"] = ug;
			POS["uj"] = uj;
			POS["ul"] = ul;
			POS["uv"] = uv;
			POS["uz"] = uz;
			POS["v"] = v;
			POS["vd"] = vd;
			POS["vg"] = vg;
			POS["vi"] = vi;
			POS["vn"] = vn;
			POS["vq"] = vq;
			POS["w"] = w;
			POS["x"] = x;
			POS["y"] = y;
			POS["yg"] = yg;
			POS["z"] = z;
			POS["zg"] = zg;
			loadModel(modelFolder);
		}
		~PosHMMModel() {
		}
		void loadModel(const string& modelFolder) {
			const string char_state_tab_path = modelFolder + "char_state_tab.utf8";
			const string prob_emit_path = modelFolder + "prob_emit.utf8";
			const string prob_start_path = modelFolder + "prob_start.utf8";
			const string prob_trans_path = modelFolder + "prob_trans.utf8";
			ifstream ifile;
			string line;
			vector<string> tmp, tmp2, tmp3;

			ifile = fsopen(char_state_tab_path);
			//load char_state_tab
			while (getLine(ifile, line)) {
				split(line, tmp, ":");
				split(tmp[1], tmp2, ";");
				UnicodeValueType unico = TransCode::decode(tmp[0])[0];
				for (auto v : tmp2) {
					split(v, tmp3, ",");
					StatusValueType statusValue = getStatusValue(status[tmp3[0][0]], POS[tmp3[1]]);
					char_state_tab[unico].push_back(statusValue);
				}
			}
			ifile.close();

			ifile = fsopen(prob_start_path);
			//load startProb
			while (getLine(ifile, line)) {
				split(line, tmp, ":");
				split(tmp[0], tmp2, ",");
				StatusValueType statusValue = getStatusValue(status[tmp2[0][0]], POS[tmp2[1]]);
				startProb[statusValue] = atof(tmp[1].c_str());
			}
			ifile.close();

			ifile = fsopen(prob_emit_path);
			//load emitProb
			while (getLine(ifile, line)) {
				split(line, tmp, ":");
				split(tmp[0], tmp3, ",");
				split(tmp[1], tmp2, ";");
				StatusValueType statusValue = getStatusValue(status[tmp3[0][0]], POS[tmp3[1]]);
				for (auto v : tmp2) {
					split(v, tmp3, ",");
					UnicodeValueType unico = TransCode::decode(tmp3[0])[0];
					emitProb[unico][statusValue] = atof(tmp3[1].c_str());
				}
			}
			ifile.close();

			ifile = fsopen(prob_trans_path);
			//load transProb
			while (getLine(ifile, line)) {
				split(line, tmp, ":");
				split(tmp[0], tmp2, ",");
				StatusValueType statusValue1 = getStatusValue(status[tmp2[0][0]], POS[tmp2[1]]);
				split(tmp[1], tmp2, ",");
				StatusValueType statusValue2 = getStatusValue(status[tmp2[0][0]], POS[tmp2[1]]);
				transProb[statusValue1][statusValue2] = atof(tmp[2].c_str());
			}
			ifile.close();
		}
		double getEmitProb(const EmitProbMapType& Mp, UnicodeValueType key1, StatusValueType key2,
			double defVal)const {
			EmitProbMapType::const_iterator cit = Mp.find(key1);
			if (cit == Mp.end()) {
				return defVal;
			}
			else {
				unordered_map<StatusValueType, double>::const_iterator citr2 = cit->second.find(key2);
				if (citr2 == cit->second.end()) {
					return defVal;
				}
				return citr2->second;
			}
		}
		bool getLine(ifstream& ifile, string& line) {
			while (getline(ifile, line)) {
				trim(line);
				if (line.empty()) {
					continue;
				}
				if (startsWith(line, "#")) {
					continue;
				}
				return true;
			}
			return false;
		}

		double startProb[ALL_STATUS_SUM];
		double transProb[ALL_STATUS_SUM][ALL_STATUS_SUM];
		EmitProbMapType emitProb;
		unordered_map<UnicodeValueType, vector<StatusValueType> > char_state_tab;
	}; // struct PosHMMModel

	char PosHMMModel::statMap[PosHMMModel::STATUS_SUM] = { 'B', 'E', 'M', 'S' };
	char PosHMMModel::posMap[64][5] = { "a", "ad", "ag", "an", "b", "bg", "c", "d", "df", "dg", "e", "en", "f", "g", "h", "i", "in", "j", "jn", "k", "l", "ln", "m", "mg", "mq", "n", "ng", "nr", "nrfg", "nrt", "ns", "nt", "nz", "o", "p", "q", "qe", "qg", "r", "rg", "rr", "rz", "s", "t", "tg", "u", "ud", "ug", "uj", "ul", "uv", "uz", "v", "vd", "vg", "vi", "vn", "vq", "w", "x", "y", "yg", "z", "zg" };

} // namespace CppJieba

#endif
