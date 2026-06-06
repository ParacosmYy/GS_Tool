#include "k20750/m20750.h"
QVector<double> m20750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
