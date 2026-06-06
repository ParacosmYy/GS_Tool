#include "g8386/m8386.h"
QVector<double> m8386::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
