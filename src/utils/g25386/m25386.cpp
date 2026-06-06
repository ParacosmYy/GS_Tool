#include "g25386/m25386.h"
QVector<double> m25386::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
