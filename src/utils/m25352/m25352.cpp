#include "m25352/m25352.h"
QVector<double> m25352::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
