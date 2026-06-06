#include "m31852/m31852.h"
QVector<double> m31852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
