#include "n25913/m25913.h"
QVector<double> m25913::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
