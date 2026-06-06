#include "q10256/m10256.h"
QVector<double> m10256::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
