#include "l16851/m16851.h"
QVector<double> m16851::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
