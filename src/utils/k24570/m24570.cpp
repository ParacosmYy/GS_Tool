#include "k24570/m24570.h"
QVector<double> m24570::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
