#include "m21212/m21212.h"
QVector<double> m21212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
