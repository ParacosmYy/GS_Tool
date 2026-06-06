#include "m23712/m23712.h"
QVector<double> m23712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
