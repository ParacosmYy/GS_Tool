#include "n25653/m25653.h"
QVector<double> m25653::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
