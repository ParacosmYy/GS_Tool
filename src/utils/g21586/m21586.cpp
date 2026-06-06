#include "g21586/m21586.h"
QVector<double> m21586::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
