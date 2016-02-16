#encoding=utf-8
from __future__ import unicode_literals
import sys
sys.path.append("../")
reload(sys)
sys.setdefaultencoding("gb18030")

import jieba
import jieba.posseg
import jieba.analyse

s = "测试字符串testing string"
words = jieba.posseg.cut(s)
for w in words:
	print '%s\%s'.encode('utf8') % (w.word, w.flag),

'''
f = open("关于语言信息处理技术的展望.txt")
of = open("关于语言信息处理技术的展望_std.txt", 'w')
for line in f:
	words = jieba.posseg.cut(line[:-1])
	for w in words:
		of.write('%s\%s '.encode('utf8') % (w.word, w.flag))
	of.write('\n'.encode('utf8'))'''