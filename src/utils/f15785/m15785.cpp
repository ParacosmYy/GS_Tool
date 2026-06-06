#include "f15785/m15785.h"
QVector<double> m15785::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
