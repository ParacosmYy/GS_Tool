#include "o8754/m8754.h"
QVector<double> m8754::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
