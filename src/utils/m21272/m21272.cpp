#include "m21272/m21272.h"
QVector<double> m21272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
