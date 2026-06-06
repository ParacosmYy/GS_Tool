#include "m32112/m32112.h"
QVector<double> m32112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
