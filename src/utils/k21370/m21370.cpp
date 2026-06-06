#include "k21370/m21370.h"
QVector<double> m21370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
