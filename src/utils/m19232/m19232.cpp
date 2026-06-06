#include "m19232/m19232.h"
QVector<double> m19232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
