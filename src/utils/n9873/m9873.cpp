#include "n9873/m9873.h"
QVector<double> m9873::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
