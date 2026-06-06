#include "g21626/m21626.h"
QVector<double> m21626::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
