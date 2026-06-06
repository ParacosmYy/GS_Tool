#include "s9258/m9258.h"
QVector<double> m9258::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
