#include "a18200/m18200.h"
QVector<double> m18200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
