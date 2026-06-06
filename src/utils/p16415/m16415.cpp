#include "p16415/m16415.h"
QVector<double> m16415::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
