#include "d19683/m19683.h"
QVector<double> m19683::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
