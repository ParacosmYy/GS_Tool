#include "a26500/m26500.h"
QVector<double> m26500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
