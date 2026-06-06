#include "m7832/m7832.h"
QVector<double> m7832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
