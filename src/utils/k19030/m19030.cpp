#include "k19030/m19030.h"
QVector<double> m19030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
