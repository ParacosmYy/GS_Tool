#include "a15960/m15960.h"
QVector<double> m15960::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
