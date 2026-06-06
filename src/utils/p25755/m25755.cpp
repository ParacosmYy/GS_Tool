#include "p25755/m25755.h"
QVector<double> m25755::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
