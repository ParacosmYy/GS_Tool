#include "k9190/m9190.h"
QVector<double> m9190::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
