#include "m21152/m21152.h"
QVector<double> m21152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
