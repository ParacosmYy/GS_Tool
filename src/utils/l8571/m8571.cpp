#include "l8571/m8571.h"
QVector<double> m8571::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
