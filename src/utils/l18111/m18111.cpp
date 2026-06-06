#include "l18111/m18111.h"
QVector<double> m18111::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
