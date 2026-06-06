#include "p16615/m16615.h"
QVector<double> m16615::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
