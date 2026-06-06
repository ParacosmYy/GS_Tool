#include "b18741/m18741.h"
QVector<double> m18741::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
