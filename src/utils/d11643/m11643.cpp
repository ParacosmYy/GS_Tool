#include "d11643/m11643.h"
QVector<double> m11643::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
