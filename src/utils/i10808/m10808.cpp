#include "i10808/m10808.h"
QVector<double> m10808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
