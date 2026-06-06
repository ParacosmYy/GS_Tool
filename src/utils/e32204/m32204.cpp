#include "e32204/m32204.h"
QVector<double> m32204::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
