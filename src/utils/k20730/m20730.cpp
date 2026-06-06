#include "k20730/m20730.h"
QVector<double> m20730::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
