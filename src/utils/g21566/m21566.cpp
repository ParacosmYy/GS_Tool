#include "g21566/m21566.h"
QVector<double> m21566::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
