#include "p8215/m8215.h"
QVector<double> m8215::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
