#include "k19650/m19650.h"
QVector<double> m19650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
