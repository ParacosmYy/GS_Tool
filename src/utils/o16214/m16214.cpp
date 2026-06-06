#include "o16214/m16214.h"
QVector<double> m16214::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
