#include "i9388/m9388.h"
QVector<double> m9388::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
