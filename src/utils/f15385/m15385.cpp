#include "f15385/m15385.h"
QVector<double> m15385::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
