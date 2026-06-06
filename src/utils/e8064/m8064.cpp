#include "e8064/m8064.h"
QVector<double> m8064::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
