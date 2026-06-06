#include "k20350/m20350.h"
QVector<double> m20350::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
