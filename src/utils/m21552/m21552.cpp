#include "m21552/m21552.h"
QVector<double> m21552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
