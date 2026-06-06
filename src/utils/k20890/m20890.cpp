#include "k20890/m20890.h"
QVector<double> m20890::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
