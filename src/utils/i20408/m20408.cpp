#include "i20408/m20408.h"
QVector<double> m20408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
