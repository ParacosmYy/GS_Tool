#include "a37100/m37100.h"
QVector<double> m37100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
