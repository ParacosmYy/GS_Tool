#include "a25820/m25820.h"
QVector<double> m25820::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
