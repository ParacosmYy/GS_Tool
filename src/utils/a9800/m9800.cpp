#include "a9800/m9800.h"
QVector<double> m9800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
