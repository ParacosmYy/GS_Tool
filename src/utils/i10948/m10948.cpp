#include "i10948/m10948.h"
QVector<double> m10948::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
