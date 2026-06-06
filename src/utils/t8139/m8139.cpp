#include "t8139/m8139.h"
QVector<double> m8139::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
