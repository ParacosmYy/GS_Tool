#include "i10128/m10128.h"
QVector<double> m10128::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
