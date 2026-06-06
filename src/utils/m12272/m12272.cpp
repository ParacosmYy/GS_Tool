#include "m12272/m12272.h"
QVector<double> m12272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
