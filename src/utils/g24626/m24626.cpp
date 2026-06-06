#include "g24626/m24626.h"
QVector<double> m24626::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
