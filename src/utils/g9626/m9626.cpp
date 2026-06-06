#include "g9626/m9626.h"
QVector<double> m9626::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
