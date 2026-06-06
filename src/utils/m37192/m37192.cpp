#include "m37192/m37192.h"
QVector<double> m37192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
