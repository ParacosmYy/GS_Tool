#include "m7812/m7812.h"
QVector<double> m7812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
