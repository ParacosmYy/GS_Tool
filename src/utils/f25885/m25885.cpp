#include "f25885/m25885.h"
QVector<double> m25885::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
