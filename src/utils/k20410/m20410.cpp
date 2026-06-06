#include "k20410/m20410.h"
QVector<double> m20410::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
