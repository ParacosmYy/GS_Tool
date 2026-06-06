#include "i9988/m9988.h"
QVector<double> m9988::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
