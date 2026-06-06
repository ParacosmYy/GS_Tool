#include "g25506/m25506.h"
QVector<double> m25506::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
