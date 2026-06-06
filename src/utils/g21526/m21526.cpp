#include "g21526/m21526.h"
QVector<double> m21526::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
