#include "n24053/m24053.h"
QVector<double> m24053::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
