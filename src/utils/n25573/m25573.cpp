#include "n25573/m25573.h"
QVector<double> m25573::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
