#include "s8518/m8518.h"
QVector<double> m8518::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
