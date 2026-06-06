#include "m32792/m32792.h"
QVector<double> m32792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
