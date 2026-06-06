#include "g21106/m21106.h"
QVector<double> m21106::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
