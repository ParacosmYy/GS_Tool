#include "a25080/m25080.h"
QVector<double> m25080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
