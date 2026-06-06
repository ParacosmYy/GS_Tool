#include "k24810/m24810.h"
QVector<double> m24810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
