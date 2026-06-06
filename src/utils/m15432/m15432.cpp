#include "m15432/m15432.h"
QVector<double> m15432::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
