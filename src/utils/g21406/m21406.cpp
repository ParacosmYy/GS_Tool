#include "g21406/m21406.h"
QVector<double> m21406::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
