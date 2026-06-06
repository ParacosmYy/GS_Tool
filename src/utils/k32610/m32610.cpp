#include "k32610/m32610.h"
QVector<double> m32610::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
