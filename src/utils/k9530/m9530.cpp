#include "k9530/m9530.h"
QVector<double> m9530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
