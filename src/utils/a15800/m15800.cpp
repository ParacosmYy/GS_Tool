#include "a15800/m15800.h"
QVector<double> m15800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
