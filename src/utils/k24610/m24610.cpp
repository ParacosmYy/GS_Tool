#include "k24610/m24610.h"
QVector<double> m24610::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
