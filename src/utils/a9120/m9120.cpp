#include "a9120/m9120.h"
QVector<double> m9120::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
