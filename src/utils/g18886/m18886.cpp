#include "g18886/m18886.h"
QVector<double> m18886::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
