#include "m23812/m23812.h"
QVector<double> m23812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
