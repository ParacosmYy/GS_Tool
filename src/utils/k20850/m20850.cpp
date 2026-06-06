#include "k20850/m20850.h"
QVector<double> m20850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
