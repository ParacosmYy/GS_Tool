#include "e12204/m12204.h"
QVector<double> m12204::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
