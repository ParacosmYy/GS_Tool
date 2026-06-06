#include "k28570/m28570.h"
QVector<double> m28570::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
