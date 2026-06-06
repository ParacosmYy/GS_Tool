#include "m19872/m19872.h"
QVector<double> m19872::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
