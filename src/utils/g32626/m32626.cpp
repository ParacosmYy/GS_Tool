#include "g32626/m32626.h"
QVector<double> m32626::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
