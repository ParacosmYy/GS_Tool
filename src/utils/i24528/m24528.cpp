#include "i24528/m24528.h"
QVector<double> m24528::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
