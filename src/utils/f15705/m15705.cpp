#include "f15705/m15705.h"
QVector<double> m15705::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
