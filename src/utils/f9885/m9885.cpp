#include "f9885/m9885.h"
QVector<double> m9885::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
