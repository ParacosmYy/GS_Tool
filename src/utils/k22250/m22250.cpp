#include "k22250/m22250.h"
QVector<double> m22250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
