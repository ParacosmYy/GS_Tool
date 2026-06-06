#include "k32230/m32230.h"
QVector<double> m32230::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
