#include "m14712/m14712.h"
QVector<double> m14712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
