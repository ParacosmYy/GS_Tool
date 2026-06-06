#include "l12511/m12511.h"
QVector<double> m12511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
