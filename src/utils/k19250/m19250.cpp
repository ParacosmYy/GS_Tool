#include "k19250/m19250.h"
QVector<double> m19250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
