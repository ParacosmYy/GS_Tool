#include "a21080/m21080.h"
QVector<double> m21080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
