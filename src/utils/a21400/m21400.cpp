#include "a21400/m21400.h"
QVector<double> m21400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
