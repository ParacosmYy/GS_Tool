#include "g35206/m35206.h"
QVector<double> m35206::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
