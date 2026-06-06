#include "f24725/m24725.h"
QVector<double> m24725::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
