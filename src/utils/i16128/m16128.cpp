#include "i16128/m16128.h"
QVector<double> m16128::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
