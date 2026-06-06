#include "b25701/m25701.h"
QVector<double> m25701::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
