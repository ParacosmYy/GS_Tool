#include "e32284/m32284.h"
QVector<double> m32284::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
