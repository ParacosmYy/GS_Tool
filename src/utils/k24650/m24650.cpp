#include "k24650/m24650.h"
QVector<double> m24650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
