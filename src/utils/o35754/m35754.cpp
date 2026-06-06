#include "o35754/m35754.h"
QVector<double> m35754::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
