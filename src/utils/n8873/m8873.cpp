#include "n8873/m8873.h"
QVector<double> m8873::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
