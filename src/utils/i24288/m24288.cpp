#include "i24288/m24288.h"
QVector<double> m24288::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
