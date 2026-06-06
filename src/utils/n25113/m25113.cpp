#include "n25113/m25113.h"
QVector<double> m25113::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
