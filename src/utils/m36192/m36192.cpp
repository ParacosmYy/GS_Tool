#include "m36192/m36192.h"
QVector<double> m36192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
