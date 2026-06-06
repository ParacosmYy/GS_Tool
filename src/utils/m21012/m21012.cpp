#include "m21012/m21012.h"
QVector<double> m21012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
