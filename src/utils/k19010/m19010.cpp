#include "k19010/m19010.h"
QVector<double> m19010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
