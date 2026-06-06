#include "l36051/m36051.h"
QVector<double> m36051::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
