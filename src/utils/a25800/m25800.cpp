#include "a25800/m25800.h"
QVector<double> m25800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
