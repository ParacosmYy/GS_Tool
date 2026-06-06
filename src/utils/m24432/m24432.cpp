#include "m24432/m24432.h"
QVector<double> m24432::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
