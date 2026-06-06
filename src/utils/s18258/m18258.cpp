#include "s18258/m18258.h"
QVector<double> m18258::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
