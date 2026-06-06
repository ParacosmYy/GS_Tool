#include "b19401/m19401.h"
QVector<double> m19401::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
