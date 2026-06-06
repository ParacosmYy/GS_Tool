#include "d25903/m25903.h"
QVector<double> m25903::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
