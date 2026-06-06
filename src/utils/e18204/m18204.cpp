#include "e18204/m18204.h"
QVector<double> m18204::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
