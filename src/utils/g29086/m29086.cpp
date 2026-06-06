#include "g29086/m29086.h"
QVector<double> m29086::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
