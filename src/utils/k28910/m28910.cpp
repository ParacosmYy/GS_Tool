#include "k28910/m28910.h"
QVector<double> m28910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
