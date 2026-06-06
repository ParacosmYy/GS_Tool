#include "k28490/m28490.h"
QVector<double> m28490::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
