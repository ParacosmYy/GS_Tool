#include "k17810/m17810.h"
QVector<double> m17810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
