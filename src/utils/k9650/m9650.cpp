#include "k9650/m9650.h"
QVector<double> m9650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
