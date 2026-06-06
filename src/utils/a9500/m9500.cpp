#include "a9500/m9500.h"
QVector<double> m9500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
