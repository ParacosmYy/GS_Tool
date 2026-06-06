#include "j7909/m7909.h"
QVector<double> m7909::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
