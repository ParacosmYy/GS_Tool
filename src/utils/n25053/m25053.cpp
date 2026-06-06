#include "n25053/m25053.h"
QVector<double> m25053::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
