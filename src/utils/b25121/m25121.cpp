#include "b25121/m25121.h"
QVector<double> m25121::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
