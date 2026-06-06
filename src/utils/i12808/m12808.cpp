#include "i12808/m12808.h"
QVector<double> m12808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
