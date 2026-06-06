#include "m10752/m10752.h"
QVector<double> m10752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
