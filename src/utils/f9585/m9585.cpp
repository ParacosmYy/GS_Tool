#include "f9585/m9585.h"
QVector<double> m9585::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
