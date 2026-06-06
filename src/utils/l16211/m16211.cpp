#include "l16211/m16211.h"
QVector<double> m16211::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
