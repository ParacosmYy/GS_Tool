#include "i25408/m25408.h"
QVector<double> m25408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
