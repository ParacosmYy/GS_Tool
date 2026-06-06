#include "m9152/m9152.h"
QVector<double> m9152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
