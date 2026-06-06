#include "m29852/m29852.h"
QVector<double> m29852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
