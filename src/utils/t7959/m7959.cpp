#include "t7959/m7959.h"
QVector<double> m7959::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
