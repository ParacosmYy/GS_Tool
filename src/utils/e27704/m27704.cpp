#include "e27704/m27704.h"
QVector<double> m27704::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
