#include "m18952/m18952.h"
QVector<double> m18952::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
