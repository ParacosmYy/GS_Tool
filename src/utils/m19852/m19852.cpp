#include "m19852/m19852.h"
QVector<double> m19852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
