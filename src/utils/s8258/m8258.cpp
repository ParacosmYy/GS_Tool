#include "s8258/m8258.h"
QVector<double> m8258::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
