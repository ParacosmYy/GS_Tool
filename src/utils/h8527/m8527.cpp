#include "h8527/m8527.h"
QVector<double> m8527::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
