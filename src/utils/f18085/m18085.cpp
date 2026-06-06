#include "f18085/m18085.h"
QVector<double> m18085::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
