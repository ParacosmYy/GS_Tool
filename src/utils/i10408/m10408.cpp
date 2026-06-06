#include "i10408/m10408.h"
QVector<double> m10408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
