#include "a7800/m7800.h"
QVector<double> m7800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
