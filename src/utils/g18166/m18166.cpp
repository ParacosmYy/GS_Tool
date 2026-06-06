#include "g18166/m18166.h"
QVector<double> m18166::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
