#include "g21386/m21386.h"
QVector<double> m21386::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
