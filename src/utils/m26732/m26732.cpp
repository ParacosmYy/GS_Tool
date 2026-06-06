#include "m26732/m26732.h"
QVector<double> m26732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
