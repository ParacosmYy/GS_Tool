#include "l18611/m18611.h"
QVector<double> m18611::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
