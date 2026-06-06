#include "g29626/m29626.h"
QVector<double> m29626::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
