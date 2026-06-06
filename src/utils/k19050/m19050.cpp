#include "k19050/m19050.h"
QVector<double> m19050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
