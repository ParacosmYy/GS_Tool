#include "c8002/m8002.h"
QVector<double> m8002::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
