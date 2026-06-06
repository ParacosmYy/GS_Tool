#include "a9900/m9900.h"
QVector<double> m9900::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
