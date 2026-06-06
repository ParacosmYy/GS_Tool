#include "m32052/m32052.h"
QVector<double> m32052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
