#include "i9288/m9288.h"
QVector<double> m9288::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
