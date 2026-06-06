#include "a24400/m24400.h"
QVector<double> m24400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
