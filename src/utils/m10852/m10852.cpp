#include "m10852/m10852.h"
QVector<double> m10852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
