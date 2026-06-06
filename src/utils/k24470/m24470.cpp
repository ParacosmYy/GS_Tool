#include "k24470/m24470.h"
QVector<double> m24470::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
