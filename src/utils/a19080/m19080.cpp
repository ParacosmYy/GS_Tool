#include "a19080/m19080.h"
QVector<double> m19080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
