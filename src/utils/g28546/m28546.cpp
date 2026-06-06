#include "g28546/m28546.h"
QVector<double> m28546::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
