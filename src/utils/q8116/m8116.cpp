#include "q8116/m8116.h"
QVector<double> m8116::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
