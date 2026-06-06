#include "m28352/m28352.h"
QVector<double> m28352::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
