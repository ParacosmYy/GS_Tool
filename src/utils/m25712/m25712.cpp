#include "m25712/m25712.h"
QVector<double> m25712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
