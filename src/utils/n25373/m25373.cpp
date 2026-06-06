#include "n25373/m25373.h"
QVector<double> m25373::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
