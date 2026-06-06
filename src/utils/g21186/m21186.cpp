#include "g21186/m21186.h"
QVector<double> m21186::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
