#include "k20610/m20610.h"
QVector<double> m20610::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
