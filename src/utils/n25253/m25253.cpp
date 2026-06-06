#include "n25253/m25253.h"
QVector<double> m25253::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
