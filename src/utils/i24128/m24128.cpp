#include "i24128/m24128.h"
QVector<double> m24128::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
