#include "g18186/m18186.h"
QVector<double> m18186::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
