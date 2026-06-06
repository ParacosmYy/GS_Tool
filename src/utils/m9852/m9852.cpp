#include "m9852/m9852.h"
QVector<double> m9852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
