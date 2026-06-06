#include "e18104/m18104.h"
QVector<double> m18104::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
