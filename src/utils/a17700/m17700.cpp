#include "a17700/m17700.h"
QVector<double> m17700::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
