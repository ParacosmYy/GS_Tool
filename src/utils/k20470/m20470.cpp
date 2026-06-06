#include "k20470/m20470.h"
QVector<double> m20470::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
