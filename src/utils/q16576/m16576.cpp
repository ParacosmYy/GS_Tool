#include "q16576/m16576.h"
QVector<double> m16576::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
