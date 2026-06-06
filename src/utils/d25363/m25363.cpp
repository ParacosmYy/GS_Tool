#include "d25363/m25363.h"
QVector<double> m25363::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
