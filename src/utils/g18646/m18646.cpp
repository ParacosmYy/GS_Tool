#include "g18646/m18646.h"
QVector<double> m18646::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
