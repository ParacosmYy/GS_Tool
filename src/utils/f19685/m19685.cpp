#include "f19685/m19685.h"
QVector<double> m19685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
