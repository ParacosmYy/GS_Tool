#include "m32492/m32492.h"
QVector<double> m32492::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
