#include "i24848/m24848.h"
QVector<double> m24848::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
