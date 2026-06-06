#include "i12408/m12408.h"
QVector<double> m12408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
