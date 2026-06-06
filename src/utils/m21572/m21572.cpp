#include "m21572/m21572.h"
QVector<double> m21572::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
