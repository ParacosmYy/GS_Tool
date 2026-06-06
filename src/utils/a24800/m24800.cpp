#include "a24800/m24800.h"
QVector<double> m24800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
