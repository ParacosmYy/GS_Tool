#include "c25002/m25002.h"
QVector<double> m25002::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
