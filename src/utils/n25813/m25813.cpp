#include "n25813/m25813.h"
QVector<double> m25813::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
