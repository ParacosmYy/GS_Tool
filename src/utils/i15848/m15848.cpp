#include "i15848/m15848.h"
QVector<double> m15848::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
