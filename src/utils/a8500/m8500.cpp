#include "a8500/m8500.h"
QVector<double> m8500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
