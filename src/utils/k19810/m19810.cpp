#include "k19810/m19810.h"
QVector<double> m19810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
