#include "m32172/m32172.h"
QVector<double> m32172::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
