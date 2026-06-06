#include "m13272/m13272.h"
QVector<double> m13272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
