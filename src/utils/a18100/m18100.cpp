#include "a18100/m18100.h"
QVector<double> m18100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
