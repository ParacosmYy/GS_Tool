#include "f24685/m24685.h"
QVector<double> m24685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
