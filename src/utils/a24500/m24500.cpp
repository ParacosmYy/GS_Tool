#include "a24500/m24500.h"
QVector<double> m24500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
