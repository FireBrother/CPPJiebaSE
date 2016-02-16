#define _CRT_SECURE_NO_WARNINGS
#pragma execution_character_set("utf-8") 
#include "src/Application.hpp"
#include "myExt/EncodingAdapter.hpp"
#include <windows.h>

#define U2G EncodingAdapter::UTF8ToGBK
#define G2U EncodingAdapter::GBKToUTF8
#define S2G EncodingAdapter::SmartToGBK
#define S2U EncodingAdapter::SmartToUTF8

using namespace CppJieba;

CppJieba::Application app("dict/jieba.dict.utf8",
		"dict/hmm_model.utf8",
		"dict/user.dict.utf8",
		"dict/pos_dict/",
		"dict/idf.utf8",
		"dict/stop_words.utf8");

const unsigned int NUM_TASKS = 48;
int total_process = 0, max_process = 0;
HANDLE mutex;
bool update = 0;
#define P(m) WaitForSingleObject(m, INFINITE);
#define V(m) ReleaseMutex(m);

bool initialize_parameter_pool(const string& walk_path, vector<vector<string> > & parameter_pool) {
    ifstream fin(walk_path.c_str());
    if (!fin) {
        cout << "Open file error" << endl;
        return false;
    }
    string line;
    int i = 0;
    parameter_pool.resize(NUM_TASKS);
    while (getline(fin, line)) {
        parameter_pool[i].push_back(line);
        i++;
        if (i == NUM_TASKS) i = 0;
    }
    return true;
}

bool postag_string(const string& s) {
	vector<pair<string, string> > tagres;
	app.tag(S2U(s), tagres);
	cout << s << endl;
	cout << tagres << endl;
	return 1;
}

bool postag_file(string filename, string outputfile_name = "") {
	string s;
	if (outputfile_name == ""){
		if (filename.substr(filename.length() - 4) == ".txt")
			outputfile_name = filename.substr(0, filename.length() - 4) + "_seg" + filename.substr(filename.length() - 4);
		else
			outputfile_name = filename + "_seg";
	}
	//cout << filename << endl << outputfile_name << endl;
	ifstream fin(S2G(filename));
	ofstream fout(S2G(outputfile_name));
	if (!fin) { printf("Open file %s failed.\n", filename.c_str()); return 0; }
	while (fin >> s) {
		vector<pair<string, string> > tagres;
		app.tag(S2U(s), tagres);
		string result = join(tagres.begin(), tagres.end(), " ");
		if (result.length() > 4 && result.substr(result.length()-4) == "/eng")
			fout << EncodingAdapter::SmartToUTF8(result) << ' ';
		else
			fout << EncodingAdapter::SmartToUTF8(result) << endl;
	}
	return 1;
}

DWORD WINAPI thread(LPVOID arg) {
	vector<string> parameters = *(vector<string> *)arg;
	for (vector<string>::iterator itr = parameters.begin(); itr != parameters.end(); ++itr) {
        string outputfile_name = itr->substr(0, 7) + "_pos" + itr->substr(11, itr->length()-5) + ".txt";
        postag_file(*itr, outputfile_name);
		P(mutex);
		update = 1;
		total_process++;
		V(mutex);
    }
    return 0;
}

bool parallel_work(string walk_path) {
    vector<vector<string> > parameter_pool;
	HANDLE handles[NUM_TASKS];
	mutex = CreateMutex(NULL, FALSE, (LPCWSTR)"mutex");
	if (!initialize_parameter_pool(walk_path, parameter_pool)) LogError("Parameter pool initialization error");
	printf("任务队列部署完成,分布在%d个线程上。\n", NUM_TASKS);
	for (int i = 0; i < NUM_TASKS; i++) {
		DWORD tid;
        handles[i] = CreateThread(0, 0, thread, LPVOID(&parameter_pool[i]), 0, &tid);
		printf("线程%lu创建成功，有%lu项任务等待中。\n", tid, parameter_pool[i].size());
		max_process += parameter_pool[i].size();
    }
	while (total_process != max_process) {
		P(mutex);
		if (update) {
			update = 0;
			printf("%d in %d finished.\r", total_process, max_process);
		}
		V(mutex);
	}
	printf("%d in %d finished.\r", total_process, max_process);
	WaitForMultipleObjects(NUM_TASKS, handles, true, INFINITE);
	return 1;
}

int main(int argc, char** argv) {	
	vector<string> words;
	if (argc != 3) {
		printf("Parameters error.\n");
		printf("Usage:  -s <string>  : Postag a string.\n");
		printf("Usage:  -f <filename>: Postag a file, the output file is <filename>_seg.\n");
		printf("Usage:  -p <filename>: Parallel mode. The paremeters should be in <filename>.\n");
		return 1;
	}
	string s(argv[2]);
	string filename(argv[2]);
	switch(argv[1][1]) {
	case 's':	
		postag_string(s);
		break;
	case 'f':
		postag_file(filename);
		break;
	case 'p':
		parallel_work(filename);
		break;
	}

	/*
	cout << "[demo] METHOD_MP" << endl;
	app.cut(s, words, METHOD_MP);
	cout << join(words.begin(), words.end(), "/") << endl;

	cout << "[demo] METHOD_HMM" << endl;
	app.cut(s, words, METHOD_HMM);
	cout << join(words.begin(), words.end(), "/") << endl;

	cout << "[demo] METHOD_MIX" << endl;
	app.cut(s, words, METHOD_MIX);
	cout << join(words.begin(), words.end(), "/") << endl;

	cout << "[demo] METHOD_FULL" << endl;
	app.cut(s, words, METHOD_FULL);
	cout << join(words.begin(), words.end(), "/") << endl;

	cout << "[demo] METHOD_QUERY" << endl;
	app.cut(s, words, METHOD_QUERY);
	cout << join(words.begin(), words.end(), "/") << endl;

	cout << "[demo] KEYWORD" << endl;
	vector<pair<string, double> > keywordres;
	app.extract(s, keywordres, 5);
	cout << s << endl;
	cout << keywordres << endl;

	cout << "[demo] Insert User Word" << endl;
	app.cut("男默女泪", words);
	cout << join(words.begin(), words.end(), "/") << endl;
	app.insertUserWord("男默女泪");
	app.cut("男默女泪", words);
	cout << join(words.begin(), words.end(), "/") << endl;
	*/
	return EXIT_SUCCESS;
}
