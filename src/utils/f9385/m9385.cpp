#include "f9385/m9385.h"
QVector<double> m9385::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
