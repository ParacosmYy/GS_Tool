#include "g21226/m21226.h"
QVector<double> m21226::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
