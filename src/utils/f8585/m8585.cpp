#include "f8585/m8585.h"
QVector<double> m8585::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
