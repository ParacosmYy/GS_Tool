#include "g18446/m18446.h"
QVector<double> m18446::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
