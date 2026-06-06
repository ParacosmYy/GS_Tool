#include "n25953/m25953.h"
QVector<double> m25953::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
