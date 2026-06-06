#include "f15405/m15405.h"
QVector<double> m15405::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
