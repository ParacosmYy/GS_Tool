#include "m21752/m21752.h"
QVector<double> m21752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
