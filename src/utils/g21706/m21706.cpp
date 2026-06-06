#include "g21706/m21706.h"
QVector<double> m21706::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
