#include "f21565/m21565.h"
QVector<double> m21565::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
