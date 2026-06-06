#include "b13101/m13101.h"
QVector<double> m13101::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
