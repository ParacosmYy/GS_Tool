#include "k20030/m20030.h"
QVector<double> m20030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
