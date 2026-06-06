#include "i10848/m10848.h"
QVector<double> m10848::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
