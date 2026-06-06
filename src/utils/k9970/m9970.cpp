#include "k9970/m9970.h"
QVector<double> m9970::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
