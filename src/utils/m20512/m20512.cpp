#include "m20512/m20512.h"
QVector<double> m20512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
