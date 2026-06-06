#include "f36685/m36685.h"
QVector<double> m36685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
