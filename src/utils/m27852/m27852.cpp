#include "m27852/m27852.h"
QVector<double> m27852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
