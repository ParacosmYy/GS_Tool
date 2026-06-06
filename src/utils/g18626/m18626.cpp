#include "g18626/m18626.h"
QVector<double> m18626::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
