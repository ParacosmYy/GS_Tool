#include "d15903/m15903.h"
QVector<double> m15903::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
