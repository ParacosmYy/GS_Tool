#include "m20112/m20112.h"
QVector<double> m20112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
