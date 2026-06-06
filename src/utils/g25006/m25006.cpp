#include "g25006/m25006.h"
QVector<double> m25006::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
