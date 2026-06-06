#include "g21906/m21906.h"
QVector<double> m21906::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
