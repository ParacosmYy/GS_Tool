#include "s18218/m18218.h"
QVector<double> m18218::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
