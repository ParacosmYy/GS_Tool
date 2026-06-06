#include "a8100/m8100.h"
QVector<double> m8100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
