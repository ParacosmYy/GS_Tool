#include "r8237/m8237.h"
QVector<double> m8237::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
