#include "p8115/m8115.h"
QVector<double> m8115::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
