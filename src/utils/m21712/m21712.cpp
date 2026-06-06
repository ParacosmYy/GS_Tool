#include "m21712/m21712.h"
QVector<double> m21712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
