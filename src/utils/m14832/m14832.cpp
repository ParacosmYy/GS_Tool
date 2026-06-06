#include "m14832/m14832.h"
QVector<double> m14832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
