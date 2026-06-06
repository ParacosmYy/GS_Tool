#include "d25043/m25043.h"
QVector<double> m25043::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
