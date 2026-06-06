#include "k15490/m15490.h"
QVector<double> m15490::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
