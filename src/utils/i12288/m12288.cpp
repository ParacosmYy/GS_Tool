#include "i12288/m12288.h"
QVector<double> m12288::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
