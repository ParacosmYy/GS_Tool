#include "a10100/m10100.h"
QVector<double> m10100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
