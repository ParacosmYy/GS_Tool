#include "r7877/m7877.h"
QVector<double> m7877::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
