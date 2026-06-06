#include "f25585/m25585.h"
QVector<double> m25585::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
