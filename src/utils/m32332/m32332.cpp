#include "m32332/m32332.h"
QVector<double> m32332::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
