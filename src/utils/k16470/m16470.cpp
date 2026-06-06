#include "k16470/m16470.h"
QVector<double> m16470::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
