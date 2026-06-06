#include "k18470/m18470.h"
QVector<double> m18470::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
