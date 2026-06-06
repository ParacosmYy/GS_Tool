#include "m37712/m37712.h"
QVector<double> m37712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
