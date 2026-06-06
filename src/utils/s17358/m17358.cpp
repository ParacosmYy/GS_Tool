#include "s17358/m17358.h"
QVector<double> m17358::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
