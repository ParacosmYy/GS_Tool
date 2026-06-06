#include "o25754/m25754.h"
QVector<double> m25754::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
